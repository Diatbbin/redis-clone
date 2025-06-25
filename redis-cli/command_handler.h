#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <vector>
#include <string>

class CommandHandler {
public:
    // Split commands into tokens
    static std::vector<std::string> splitCommand(const std::string &input);
private:
};

#endif