#include "Connection.h"

#include <muduo/base/Logging.h>

#include <functional>

#include "ConnectionPool.h"

using namespace lava::hiredis;
using namespace muduo;

Connection::Connection(muduo::net::EventLoop* loop,
                       const muduo::net::InetAddress& serverAddr,
                       ConnectionPool* pool)
    : hiredis_(loop, serverAddr), pool_(pool), state_(kDisconnected) {
  hiredis_.setConnectCallback(
      std::bind(&Connection::connectCallback, this, _1, _2));
  hiredis_.setDisconnectCallback(
      std::bind(&Connection::disconnectCallback, this, _1, _2));
}

Connection::~Connection() {}

void Connection::execute(const Hiredis::CommandCallback& cb,
                         const std::string& cmd) {
  switch (state_) {
    case kDisconnected:
      commandCallback_ = std::move(cb);
      cmd_ = std::move(cmd);
      state_ = kConnecting;
      connect();
      break;
    case kReady:
      command(cb, cmd);
      break;
    default:
      break;
  }
}

void Connection::disconnect() { hiredis_.disconnect(); }

void Connection::connect() { hiredis_.connect(); }

void Connection::auth() {
  hiredis_.command(std::bind(&Connection::authCallback, this, _1, _2),
                   "AUTH %s", pool_->password().c_str());
}

void Connection::select() {
  hiredis_.command(std::bind(&Connection::selectCallback, this, _1, _2),
                   "SELECT %d", pool_->index());
}

void Connection::command(const Hiredis::CommandCallback& cb,
                         const std::string& cmd) {
  hiredis_.command(cb, cmd.c_str());
}

void Connection::connectCallback(Hiredis* c, int status) {
  assert(state_ == kConnecting);

  if (status != REDIS_OK) {
    LOG_ERROR << "connectCallback Error:" << c->errstr();
    state_ = kDisconnected;
    commandCallback_(c, nullptr);
    return;
  }
  LOG_INFO << "Connected...";
  state_ = kAuthing;
  auth();
}

void Connection::authCallback(hiredis::Hiredis* c, redisReply* reply) {
  assert(state_ == kAuthing);

  if (reply->type == REDIS_REPLY_ERROR) {
    commandCallback_(c, reply);
    return;
  }
  state_ = kSelecting;
  select();
}

void Connection::selectCallback(hiredis::Hiredis* c, redisReply* reply) {
  assert(state_ == kSelecting);

  if (reply->type == REDIS_REPLY_ERROR) {
    commandCallback_(c, reply);
    return;
  }
  state_ = kReady;
  command(commandCallback_, cmd_);
}

void Connection::disconnectCallback(Hiredis* c, int status) {
  if (status != REDIS_OK) {
    LOG_ERROR << "disconnectCallback Error:" << c->errstr();
  } else {
    LOG_INFO << "Disconnected...";
  }
  state_ = kDisconnected;
}
