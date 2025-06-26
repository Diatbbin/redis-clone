#ifndef RESPONSE_PARSER_H
#define RESPONSE_PARSER_H

#include <string>

class ResponseParser {
public:
    // Read from the given socket and return parsed response as a string
    // If it fails, return empty string
    static std::string parseResponse(int sockfd);  

private:
    // Redis Serialization Protocol 2
    static std::string parseSimpleString(int sockfd);
    static std::string parseSimpleError(int sockfd);
    static std::string parseInteger(int sockfd);
    static std::string parseBulkString(int sockfd);
    static std::string parseArray(int sockfd);
};


#endif