#ifndef REDIS_SERVER_H
#define REDIS_SERVER_H

#include <atomic>
#include <string>

class RedisServer {
public:
    RedisServer(int port);
    void run();
    void shutdown();

private:
    int port;
    int server_socket;
    std::atomic<bool> isRunning;

    // Setup s  ignal to handle graceful shutdown (ctrl + c)
    void setupSignalHandler();
};

#endif