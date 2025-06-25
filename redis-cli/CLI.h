#ifndef CLI_H
#define CLI_H

#include <string>
#include <iostream>
#include "redis_client.h"
#include "command_handler.h"

class CLI {
public:
    CLI(const std::string &host, int port);
    void run();

private:
    RedisClient redisClient;

};

#endif