#include "HiredisPool.h"

#include "Hiredis.h"

using namespace lava::hiredis;

HiredisPool::HiredisPool(const muduo::net::InetAddress& serverAddr,
                         const std::string& password__, int index__,
                         size_t maxPoolSize)
    : serverAddr_(serverAddr),
      password_(password__),
      index_(index__),
      maxPoolSize_(maxPoolSize) {}

HiredisPool::~HiredisPool() { assert(pool_.empty()); }

HiredisPtr HiredisPool::get(muduo::net::EventLoop* loop) {
  loop->assertInLoopThread();
  auto it = pool_.find(loop);
  if (it == pool_.end()) {
    {
      muduo::MutexLockGuard lock(mutex_);
      pool_[loop] = std::deque<HiredisPtr>();
    }

    it = pool_.find(loop);
    assert(it != pool_.end());
  }

  HiredisPtr c;
  if (it->second.empty()) {
    c.reset(new Hiredis(loop, serverAddr_));
    return c;
  }
  c = it->second.front();
  it->second.pop_front();

  return c;
}

void HiredisPool::put(const HiredisPtr& c) {
  muduo::net::EventLoop* loop = c->getLoop();
  loop->assertInLoopThread();
  auto it = pool_.find(loop);
  assert(it != pool_.end());
  if (it->second.size() >= maxPoolSize_) {
    return;
  }
  it->second.push_back(c);
}

//必须保证请求都处理完
void HiredisPool::clear() {
  for (auto& i : pool_) {
    for (auto& c : i.second) {
      i.first->runInLoop(std::bind(&Hiredis::disconnect, c));
    }
    i.second.clear();
  }

  pool_.clear();
}
