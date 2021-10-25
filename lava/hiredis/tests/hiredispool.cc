#include <lava/hiredis/ConnectionPool.h>
#include <lava/hiredis/Request.h>
#include <muduo/base/CountDownLatch.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThreadPool.h>

using namespace lava;
using namespace muduo;
using namespace muduo::net;

string toString(long long value) {
  char buf[32];
  snprintf(buf, sizeof buf, "%lld", value);
  return buf;
}

string redisReplyToString(const redisReply* reply) {
  static const char* const types[] = {"",
                                      "REDIS_REPLY_STRING",
                                      "REDIS_REPLY_ARRAY",
                                      "REDIS_REPLY_INTEGER",
                                      "REDIS_REPLY_NIL",
                                      "REDIS_REPLY_STATUS",
                                      "REDIS_REPLY_ERROR"};
  string str;
  if (!reply) return str;

  str += types[reply->type] + string("(") + toString(reply->type) + ") ";

  str += "{ ";
  if (reply->type == REDIS_REPLY_STRING || reply->type == REDIS_REPLY_STATUS ||
      reply->type == REDIS_REPLY_ERROR) {
    str += '"' + string(reply->str, reply->len) + '"';
  } else if (reply->type == REDIS_REPLY_INTEGER) {
    str += toString(reply->integer);
  } else if (reply->type == REDIS_REPLY_ARRAY) {
    str += toString(reply->elements) + " ";
    for (size_t i = 0; i < reply->elements; i++) {
      str += " " + redisReplyToString(reply->element[i]);
    }
  }
  str += " }";

  return str;
}

void commandCallback1(hiredis::Hiredis* c, redisReply* reply,
                      hiredis::Request* req) {
  LOG_INFO << "time1 " << redisReplyToString(reply);
  delete req;
}

void commandCallback2(hiredis::Hiredis* c, redisReply* reply,
                      hiredis::Request* req, muduo::CountDownLatch* latch,
                      int* type) {
  LOG_INFO << "time2 " << redisReplyToString(reply);
  *type = reply->type;
  latch->countDown();
  delete req;
}

int main(int argc, char** argv) {
  Logger::setLogLevel(Logger::DEBUG);

  EventLoop loop;
  EventLoopThreadPool threadPool(&loop, "test");
  threadPool.setThreadNum(1);
  threadPool.start();

  InetAddress serverAddr("172.29.233.88", 6379);
  hiredis::ConnectionPool hiredisPool(serverAddr, "123456", 15);

  //异步方式
  hiredis::Request* req1 =
      new hiredis::Request(threadPool.getNextLoop(), &hiredisPool); // FIX 传EventLoopThreadPool
  req1->execute(std::bind(commandCallback1, _1, _2, req1), "time"); // FIX 在EventLoopThreadPool::baseLoop里threadPool.getNextLoop()

  //半同步半异步方式
  muduo::CountDownLatch latch(1);
  int type = 0;
  hiredis::Request* req2 =
      new hiredis::Request(threadPool.getNextLoop(), &hiredisPool);
  req2->execute(std::bind(commandCallback2, _1, _2, req2, &latch, &type),
                "time");
  latch.wait();
  LOG_DEBUG << "type: " << type;

  loop.loop();

  return 0;
}
