# Build my own Redis Client and Server

This project is built to learn about **Redis internals** and **socket programming in C++**.

To get redis server executable, in root directory
run 

`make rebuild`

To get redis client executable
First, cd to redis-cli directory using

`cd redis-cli`

Then run 

`make rebuild`

Run the server or client using 

`./my_redis_server` or `./my_redis_cli`

In-memory key-value store

# Implementation details

Basic Redis commands support for Key value, List and Hash operations

RESP (REdis Serialization Protocol) parser and serializer

Socket-based client-server communication

Thread-safe operations using std::mutex
