#ifndef LAVA_HIREDIS_HIREDISREQUEST_H
#define LAVA_HIREDIS_HIREDISREQUEST_H

#include <muduo/base/noncopyable.h>

#include "Hiredis.h"

namespace muduo {
namespace net {
class EventLoop;
}
}  // namespace muduo

namespace lava {
namespace hiredis {
class HiredisPool;
class HiredisRequest : muduo::noncopyable {
 public:
  HiredisRequest(HiredisPool* pool, muduo::net::EventLoop* loop);
  ~HiredisRequest();

  void command(const Hiredis::CommandCallback& cb, const std::string& cmd);

 private:
  void commandInLoop();

  void init();
  void connect();
  void auth();
  void select();
  void command();

  void connectCallback(Hiredis* c, int status);
  void disconnectCallback(Hiredis* c, int status);
  void authCallback(Hiredis* c, redisReply* reply);
  void selectCallback(Hiredis* c, redisReply* reply);
  void commandCallback(Hiredis* c, redisReply* reply);

  enum State { kDisconnected, kConnecting, kAuthing, kSelecting, kReady };

  State state_ = kDisconnected;

  HiredisPool* pool_;
  muduo::net::EventLoop* loop_;
  HiredisPtr hiredis_;

  Hiredis::CommandCallback cb_;
  std::string cmd_;
};
}  // namespace hiredis
}  // namespace lava

#endif
