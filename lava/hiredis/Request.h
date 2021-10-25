#ifndef LAVA_HIREDIS_REQUEST_H
#define LAVA_HIREDIS_REQUEST_H

#include "Connection.h"

namespace muduo {
namespace net {
class EventLoop;
}  // namespace net
}  // namespace muduo

namespace lava {
namespace hiredis {

class ConnectionPool;
class Request : muduo::noncopyable {
 public:
  Request(muduo::net::EventLoop* loop, ConnectionPool* pool);
  ~Request();

  void execute(const Hiredis::CommandCallback& cb, const std::string& cmd);

 private:
  void executeInLoop(const Hiredis::CommandCallback& cb,
                     const std::string& cmd);
  void executeCallback(Hiredis* c, redisReply* reply,
                       const Hiredis::CommandCallback& cb);

  muduo::net::EventLoop* loop_;
  ConnectionPool* pool_;
  ConnectionPtr connection_;
};

}  // namespace hiredis
}  // namespace lava

#endif
