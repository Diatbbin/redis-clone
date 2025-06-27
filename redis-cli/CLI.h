#ifndef CLI_H
#define CLI_H

#include <string>
#include <iostream>
#include "redis_client.h"
#include "command_handler.h"
#include "response_parser.h"

class CLI {
public:
    CLI(const std::string &host, int port);
    void run(const std::vector<std::string>& args);
    void executeCommand(const std::vector<std::string>& args);

private:
    std::string host;
    int port;
    RedisClient redisClient;
};

#endif