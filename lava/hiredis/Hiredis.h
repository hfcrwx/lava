// Adapted from muduo contrib/hiredis/Hiredis.h

#ifndef LAVA_HIREDIS_HIREDIS_H
#define LAVA_HIREDIS_HIREDIS_H

#include <hiredis/hiredis.h>
#include <muduo/base/StringPiece.h>
#include <muduo/base/noncopyable.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/InetAddress.h>

struct redisAsyncContext;
struct redisReply;

namespace muduo {
namespace net {
class Channel;
class EventLoop;
}  // namespace net
}  // namespace muduo

namespace lava {
namespace hiredis {

class Hiredis : muduo::noncopyable {
 public:
  typedef std::function<void(Hiredis*, int)> ConnectCallback;
  typedef std::function<void(Hiredis*, int)> DisconnectCallback;
  typedef std::function<void(Hiredis*, redisReply*)> CommandCallback;

  Hiredis(muduo::net::EventLoop* loop,
          const muduo::net::InetAddress& serverAddr);
  ~Hiredis();

  muduo::net::EventLoop* getLoop() const { return loop_; }
  const muduo::net::InetAddress& serverAddress() const { return serverAddr_; }

  bool connected() const;
  const char* errstr() const;

  void setConnectCallback(const ConnectCallback& cb) { connectCb_ = cb; }
  void setDisconnectCallback(const DisconnectCallback& cb) {
    disconnectCb_ = cb;
  }

  void connect();
  void disconnect();
  int command(const CommandCallback& cb, muduo::StringArg cmd, ...);

 private:
  void setChannel();
  void removeChannel();

  void handleRead(muduo::Timestamp receiveTime);
  void handleWrite();

  void connectCallback(int status);
  void disconnectCallback(int status);
  void commandCallback(redisReply* reply, CommandCallback* cb);

  int fd() const;
  void logConnection(bool up) const;

  static Hiredis* getHiredis(const redisAsyncContext* ac);

  static void connectCallback(const redisAsyncContext* ac, int status);
  static void disconnectCallback(const redisAsyncContext* ac, int status);
  static void commandCallback(redisAsyncContext* ac, void* r, void* privdata);

  static void addRead(void* privdata);
  static void delRead(void* privdata);
  static void addWrite(void* privdata);
  static void delWrite(void* privdata);
  static void cleanup(void* privdata);

 private:
  muduo::net::EventLoop* loop_;
  const muduo::net::InetAddress serverAddr_;
  redisAsyncContext* context_;
  std::shared_ptr<muduo::net::Channel> channel_;
  ConnectCallback connectCb_;
  DisconnectCallback disconnectCb_;
};

typedef std::shared_ptr<Hiredis> HiredisPtr;

}  // namespace hiredis
}  // namespace lava

#endif  // LAVA_HIREDIS_HIREDIS_H
