#include "Request.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include "ConnectionPool.h"

using namespace muduo;
using namespace lava::hiredis;

Request::Request(muduo::net::EventLoop* loop, ConnectionPool* pool)
    : loop_(loop), pool_(pool) {}

Request::~Request() { loop_->assertInLoopThread(); }

void Request::execute(const Hiredis::CommandCallback& cb,
                      const std::string& cmd) {
  loop_->runInLoop(std::bind(&Request::executeInLoop, this, cb, cmd));
}

void Request::executeInLoop(const Hiredis::CommandCallback& cb,
                            const std::string& cmd) {
  // loop = getNextLoop
  loop_->assertInLoopThread();

  connection_ = pool_->get(loop_); // loop
  connection_->execute(std::bind(&Request::executeCallback, this, _1, _2, cb),
                       cmd);
}

void Request::executeCallback(hiredis::Hiredis* c, redisReply* reply,
                              const Hiredis::CommandCallback& cb) {
  pool_->put(connection_);
  cb(c, reply);
}
