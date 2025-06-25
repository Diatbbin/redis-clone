#ifndef REDIS_CLIENT_H
#define REDIS_CLIENT_H

#include <string>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

class RedisClient {
public: 
    RedisClient(const std::string &host, int port);
    ~RedisClient();

    bool connectToServer();
    void disconnect();
    int getSocketFD() const;

private:
    std::string host;
    int port;
    int sockfd;
};

#endif