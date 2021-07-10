#ifndef LAVA_HIREDIS_HIREDISPOOL_H
#define LAVA_HIREDIS_HIREDISPOOL_H

#include "Hiredis.h"

#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>

#include <map>
#include <memory>
#include <deque>

namespace lava {
namespace hiredis {
class HiredisPool : muduo::noncopyable {
 public:
  HiredisPool(const muduo::net::InetAddress& serverAddr,
              const std::string& password, int index,
              size_t maxPoolSize = 1024);
  ~HiredisPool();

  HiredisPtr get(muduo::net::EventLoop* loop);
  void put(const HiredisPtr& c);
  void clear();

  const std::string& password() const { return password_; }
  int index() const { return index_; }

 private:
  const muduo::net::InetAddress serverAddr_;
  const std::string password_;
  const int index_;
  size_t maxPoolSize_;

  muduo::MutexLock mutex_;
  std::map<muduo::net::EventLoop*, std::deque<HiredisPtr>> pool_;
};
}  // namespace hiredis
}  // namespace lava

#endif
