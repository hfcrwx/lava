#ifndef LAVA_HIREDIS_CONNECTION_H
#define LAVA_HIREDIS_CONNECTION_H

#include "Hiredis.h"

namespace lava {
namespace hiredis {

class ConnectionPool;
class Connection : muduo::noncopyable {
 public:
  Connection(muduo::net::EventLoop* loop,
             const muduo::net::InetAddress& serverAddr, ConnectionPool* pool);
  ~Connection();

  muduo::net::EventLoop* getLoop() const { return hiredis_.getLoop(); }

  void execute(const Hiredis::CommandCallback& cb, const std::string& cmd);
  void disconnect();

 private:
  void connect();
  void auth();
  void select();
  void command(const Hiredis::CommandCallback& cb, const std::string& cmd);

  void connectCallback(Hiredis* c, int status);
  void authCallback(Hiredis* c, redisReply* reply);
  void selectCallback(Hiredis* c, redisReply* reply);
  void disconnectCallback(Hiredis* c, int status);

 private:
  enum States { kDisconnected, kConnecting, kAuthing, kSelecting, kReady };

  Hiredis hiredis_;
  ConnectionPool* pool_;
  States state_ = kDisconnected;

  Hiredis::CommandCallback commandCallback_;
  std::string cmd_;
};

typedef std::shared_ptr<Connection> ConnectionPtr;

}  // namespace hiredis
}  // namespace lava

#endif
