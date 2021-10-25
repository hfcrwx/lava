#include "ConnectionPool.h"

#include <muduo/net/EventLoop.h>

using namespace lava::hiredis;

ConnectionPool::ConnectionPool(const muduo::net::InetAddress& serverAddr,
                               const std::string& password__, int index__,
                               size_t size)
    : serverAddr_(serverAddr),
      password_(password__),
      index_(index__),
      size_(size) {}

ConnectionPool::~ConnectionPool() { assert(pool_.empty()); }

ConnectionPtr ConnectionPool::get(muduo::net::EventLoop* loop) {
  loop->assertInLoopThread();
  auto it = pool_.find(loop);
  if (it == pool_.end()) {
    {
      muduo::MutexLockGuard lock(mutex_);
      pool_[loop] = std::deque<ConnectionPtr>();
    }

    it = pool_.find(loop);
    assert(it != pool_.end());
  }

  ConnectionPtr c;
  if (it->second.empty()) {
    c.reset(new Connection(loop, serverAddr_, this));
    return c;
  }
  c = it->second.front();
  it->second.pop_front();

  return c;
}

void ConnectionPool::put(const ConnectionPtr& c) {
  muduo::net::EventLoop* loop = c->getLoop();
  loop->assertInLoopThread();
  auto it = pool_.find(loop);
  assert(it != pool_.end());
  if (it->second.size() >= size_) {
    return;
  }
  it->second.push_back(c);
}

//必须保证请求都处理完
void ConnectionPool::clear() {
  for (auto& i : pool_) {
    for (auto& c : i.second) {
      i.first->runInLoop(std::bind(&Connection::disconnect, c));
    }
    i.second.clear();
  }

  pool_.clear();
}
