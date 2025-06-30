#ifndef REDIS_COMMAND_HANDLER
#define REDIS_COMMAND_HANDLER

#include <string>
#include <typeinfo>

class RedisCommandHandler {
public:
    RedisCommandHandler();

    std::string processCommand(const std::string& commandLine);
};

#endif