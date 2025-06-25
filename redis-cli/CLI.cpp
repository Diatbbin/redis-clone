#include "CLI.h"

CLI::CLI(const std::string &host, int port): redisClient(host, port) {}

void CLI::run() {
    if (!redisClient.connectToServer()) {
        return;
    }

    std::cout << "Connection successful at \n"; 
    // std::string host = "127.0.0.1";
    // int port = 6379;
}