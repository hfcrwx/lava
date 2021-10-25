#ifndef LAVA_HIREDIS_CONNECTIONPOOL_H
#define LAVA_HIREDIS_CONNECTIONPOOL_H

#include <muduo/base/Mutex.h>

#include <deque>
#include <map>

#include "Connection.h"

namespace muduo {
namespace net {
class EventLoop;
}  // namespace net
}  // namespace muduo

namespace lava {
namespace hiredis {

class ConnectionPool : muduo::noncopyable {
 public:
  ConnectionPool(const muduo::net::InetAddress& serverAddr,
                 const std::string& password, int index,
                 size_t size = 1024);
  ~ConnectionPool();

  ConnectionPtr get(muduo::net::EventLoop* loop);
  void put(const ConnectionPtr& c);
  void clear();

  const std::string& password() const { return password_; }
  int index() const { return index_; }

 private:
  const muduo::net::InetAddress serverAddr_;
  const std::string password_;
  const int index_;
  size_t size_;

  muduo::MutexLock mutex_;
  std::map<muduo::net::EventLoop*, std::deque<ConnectionPtr>> pool_;
};

}  // namespace hiredis
}  // namespace lava

#endif
