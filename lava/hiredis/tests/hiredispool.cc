#include <string>

#include "lava/hiredis/Hiredis.h"
#include "lava/hiredis/HiredisPool.h"
#include "lava/hiredis/HiredisRequest.h"
#include <muduo/base/CountDownLatch.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include <muduo/net/EventLoopThreadPool.h>

using namespace muduo;
using namespace muduo::net;
using namespace lava;

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

void commandCallback1(hiredis::Hiredis* c, redisReply* reply) {
  LOG_INFO << "time1 " << redisReplyToString(reply);
}

void commandCallback2(hiredis::Hiredis* c, redisReply* reply, muduo::CountDownLatch* latch, int* type) {
  LOG_INFO << "time2 " << redisReplyToString(reply);
  *type = reply->type;
  latch->countDown();
}

int main(int argc, char** argv) {
  Logger::setLogLevel(Logger::DEBUG);

  EventLoop loop;
  EventLoopThreadPool threadPool(&loop, "test");
  threadPool.setThreadNum(1);
  threadPool.start();

  InetAddress serverAddr("172.29.233.88", 6379);
  hiredis::HiredisPool hiredisPool(serverAddr, "123456", 15);

  //异步方式
  hiredis::HiredisRequest request1(&hiredisPool, threadPool.getNextLoop());
  request1.command(std::bind(commandCallback1, _1, _2), "time");

  //半同步半异步方式
  muduo::CountDownLatch latch(1);
  int type = 0;
  hiredis::HiredisRequest request2(&hiredisPool, threadPool.getNextLoop());
  request2.command(std::bind(commandCallback2, _1, _2, &latch, &type), "time");
  latch.wait();
  LOG_DEBUG << "type: " << type;

  loop.loop();

  return 0;
}
