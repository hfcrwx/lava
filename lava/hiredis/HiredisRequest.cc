#include "HiredisRequest.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include "HiredisPool.h"

using namespace muduo;
using namespace lava::hiredis;

HiredisRequest::HiredisRequest(HiredisPool* pool, muduo::net::EventLoop* loop)
    : pool_(pool), loop_(loop) {}

HiredisRequest::~HiredisRequest() { loop_->assertInLoopThread(); }

void HiredisRequest::command(const Hiredis::CommandCallback& cb,
                             const std::string& cmd) {
  cb_ = std::move(cb);
  cmd_ = std::move(cmd);
  loop_->runInLoop(std::bind(&HiredisRequest::commandInLoop, this));
}

void HiredisRequest::commandInLoop() {
  loop_->assertInLoopThread();

  switch (state_) {
    case kDisconnected:
      init();
      connect();
      state_ = kConnecting;
      break;
    case kReady:
      command();
      break;
    default:
      break;
  }
}

void HiredisRequest::init() {
  hiredis_ = pool_->get(loop_);
  hiredis_->setConnectCallback(
      std::bind(&HiredisRequest::connectCallback, this, _1, _2));
  hiredis_->setDisconnectCallback(
      std::bind(&HiredisRequest::disconnectCallback, this, _1, _2));
}

void HiredisRequest::connect() { hiredis_->connect(); }

void HiredisRequest::auth() {
  hiredis_->command(std::bind(&HiredisRequest::authCallback, this, _1, _2),
                    "AUTH %s", pool_->password().c_str());
}

void HiredisRequest::select() {
  hiredis_->command(std::bind(&HiredisRequest::selectCallback, this, _1, _2),
                    "SELECT %d", pool_->index());
}

void HiredisRequest::command() {
  hiredis_->command(std::bind(&HiredisRequest::commandCallback, this, _1, _2),
                    cmd_.c_str());
}

void HiredisRequest::connectCallback(Hiredis* c, int status) {
  if (status != REDIS_OK) {
    LOG_ERROR << "connectCallback Error:" << c->errstr();
    commandCallback(c, nullptr);
    return;
  }
  LOG_INFO << "Connected...";
  assert(state_ == kConnecting);
  auth();
  state_ = kAuthing;
}

void HiredisRequest::disconnectCallback(Hiredis* c, int status) {
  if (status != REDIS_OK) {
    LOG_ERROR << "disconnectCallback Error:" << c->errstr();
  } else {
    LOG_INFO << "Disconnected...";
  }
}

void HiredisRequest::authCallback(hiredis::Hiredis* c, redisReply* reply) {
  if (reply->type == REDIS_REPLY_ERROR) {
    commandCallback(c, reply);
    return;
  }
  assert(state_ == kAuthing);
  select();
  state_ = kSelecting;
}

void HiredisRequest::selectCallback(hiredis::Hiredis* c, redisReply* reply) {
  if (reply->type == REDIS_REPLY_ERROR) {
    commandCallback(c, reply);
    return;
  }
  assert(state_ == kSelecting);
  state_ = kReady;
  command();
}

void HiredisRequest::commandCallback(hiredis::Hiredis* c, redisReply* reply) {
  pool_->put(hiredis_);
  cb_(c, reply);
}
