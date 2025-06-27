#include "CLI.h"
#include <vector>

// Helper function to remove whitespaces
static std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r\f\v");
    return s.substr(start, end - start + 1);
}

CLI::CLI(const std::string &host, int port)
    : host(host), port(port), redisClient(host, port) {}

void CLI::run(const std::vector<std::string>& args) {
    if (!redisClient.connectToServer()) {
        return;
    }

    if (!args.empty()) {
        executeCommand(args);
    }
    
    std::cout << "Connection successful at " << redisClient.getSocketFD() << "\n"; 
    std::string host = "127.0.0.1";
    int port = 6379;

    while (true) {
        std::cout << host << ":" << port << "> ";
        std::cout.flush();
        std::string line;
        if (!std::getline(std::cin, line)) break;
        line = trim(line);

        if (line.empty()) continue;
        if (line == "quit" || line == "help") {
            std::cout << "Exiting redis client\n";
            break;
        }

        if (line == "help") {
            std::cout << "Display additional information\n";
            continue;
        }

        // Split command into tokens
        std::vector<std::string> args = CommandHandler::splitCommand(line);
        if (args.empty()) continue;

        std::string command = CommandHandler::buildRESPCommands(args);
        if (!redisClient.sendCommand(command)) {
            std::cerr << "(Error) Failed to send command\n";
            break;
        }

        std::string response = ResponseParser::parseResponse(redisClient.getSocketFD());
        std::cout << response << "\n";
    }
    redisClient.disconnect();
}

void CLI::executeCommand(const std::vector<std::string>& args) {
    if (args.empty()) return;

    std::string command = CommandHandler::buildRESPCommands(args);
    if (!redisClient.sendCommand(command)) {
        std::cerr << "(Error) failed to send command.\n";
        return;
    }

    // Parse and print response 
    std::string response = ResponseParser::parseResponse(redisClient.getSocketFD());
    std::cout << response << "\n";
}