#ifndef REDIS_DATABASE_H
#define REDIS_DATABASE_H

#include <string>
#include <mutex>
#include <unordered_map>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>

class RedisDatabase {
public:
    // Get the singleton database
    static RedisDatabase& getInstance();

    // Common commands
    bool flushAll();

    // Key value operations
    void set(const std::string& key, const std::string& val);
    bool get(const std::string& key, const std::string& val);
    std::vector<std::string> keys();
    std::string type(const std::string& key);
    bool del(const std::string& key);

    // Expire
    bool expire(const std::string& key, int sec);

    // Rename 
    bool rename(const std::string& oldKey, const std::string& newKey);

    // Persistance: Dump/Load the db from a file
    bool dump(const std::string& filename);
    bool load(const std::string& filename);

private:
    RedisDatabase() = default;
    ~RedisDatabase() = default;
    RedisDatabase(const RedisDatabase& ) = delete;
    RedisDatabase& operator=(const RedisDatabase&) = delete;

    std::mutex db_mutex;
    std::unordered_map<std::string, std::string> kv_store;
    std::unordered_map<std::string, std::vector<std::string>> list_store;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> hash_store;

    std::unordered_map<std::string, std::chrono::steady_clock::time_point> expiry_map;
};

#endif

