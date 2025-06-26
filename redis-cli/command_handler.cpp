#include "command_handler.h"

#include <regex>

std::vector<std::string> CommandHandler::splitCommand(const std::string &input) {
    std::vector<std::string> tokens;

    // Regex to match non whitespace words or quoted strings
    std::regex rgx(R"((\"[^\"]+\"|\S+))");
    auto words_begin = std::sregex_iterator(input.begin(), input.end(), rgx);
    auto words_end = std::sregex_iterator();

    for (auto it = words_begin; it != words_end; ++it) {
        std::string token = it->str();

        // Remove quotes
        if (token.size()) {
            if (token.size() > 2 && token.front() == '\"' && token.back() == '\"') {
                token = token.substr(1, token.size()-2);
            }
        }
        tokens.push_back(token);
    }

    return tokens;  
}

std::string CommandHandler::buildRESPCommands (const std::vector<std::string> &args) {
    std::ostringstream oss;
    oss << "*" << args.size() << "\r\n";

    for (const auto &arg : args) {
        oss << "$" << arg.size() << "\r\n" << arg << "\r\n";
    }
    return oss.str();
}