#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <vector>
#include <string>
#include <sstream>

class CommandHandler {
public:
    // Split commands into tokens
    static std::vector<std::string> splitCommand(const std::string &input);

    // Build a RESP command from the vector arguments
    static std::string buildRESPCommands(const std::vector<std::string> &args);
private:
};

#endif