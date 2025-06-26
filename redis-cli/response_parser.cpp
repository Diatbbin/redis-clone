#include "response_parser.h"

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <cstdlib>
#include <sys/socket.h>

// Function to read a single char from the socket
static bool readChar(int sockfd, char &c) {
    ssize_t r = recv(sockfd, &c, 1, 0);
    return r == 1;
}

// Function to read a line of text from socket
std::string readLine(int sockfd) {
    std::string line;
    char c;
    while (readChar(sockfd, c)) {
        if (c == '\r') {
            readChar(sockfd, c);
            break;
        }
        line.push_back(c);
    }
    return line;
}

std::string ResponseParser::parseResponse(int sockfd) {
    char prefix;

    if (!readChar(sockfd, prefix)) {
        return "Error: no response or connection close";
    }

    switch (prefix) {
        case '+': return parseSimpleString(sockfd);
        case '-': return parseSimpleError(sockfd);
        case ':': return parseInteger(sockfd);
        case '$': return parseBulkString(sockfd);
        case '*': return parseArray(sockfd);
        default:
            return "Error: unknown reply type";
    }
}

std::string ResponseParser::parseSimpleString(int sockfd) {
    return readLine(sockfd);
}

std::string ResponseParser::parseSimpleError(int sockfd) {
    
}

std::string ResponseParser::parseInteger(int sockfd) {
 
}

std::string ResponseParser::parseBulkString(int sockfd) {
   
}

std::string ResponseParser::parseArray(int sockfd) {
 
}
