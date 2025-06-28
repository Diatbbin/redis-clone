#include "redis_database.h"

#include <fstream>

RedisDatabase& RedisDatabase::getInstance() {
    static RedisDatabase instance;
    return instance;
}

/*
Memory -> file - dump()
file -> memory - load()
*/

bool RedisDatabase::dump(const std::string& filename) {
    std::lock_guard<std::mutex> lock(db_mutex);
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) return false;

    for (const auto& kv : kv_store) {
        ofs << "K" << kv.first << " " << kv.second << "\n";
    }

    for (const auto& kv : list_store) {
        ofs << "L" << kv.first;

        for (const auto& item : kv.second) {
            ofs << " " << item;
        }
        ofs << "\n";
        
    }

    for (const auto& kv : hash_store) {
        ofs << "H " << kv.first;
        for (const auto& field_val : kv.second) {
            ofs << " " << field_val.first << ":" << field_val.second;
        }
        ofs << "\n";
    }

    return true;
}

bool RedisDatabase::load(const std::string& filename) {
    std::lock_guard<std::mutex> lock(db_mutex);

    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) return false;

    kv_store.clear();
    list_store.clear();
    hash_store.clear();

    std::string line;
    while (std::getline(ifs, line)) {
        std::istringstream iss(line);
        char type;
        iss >> type;
        if (type == 'K') {
            std::string key, value;
            iss >> key >> value;
            kv_store[key] = value;
        } else if (type == 'L') {
            std::string key;
            iss >> key;
            std::string item;
            std::vector<std::string> list;
            while (iss >> item) {
                list.push_back(item);
            }

            list_store[key] = list;
        } else if (type == 'H') {
            std::string key;
            std::unordered_map<std::string, std::string> hash;
            std::string pr;
            while (iss >> pr) {
                auto pos = pr.find(':');
                if (pos != std::string::npos) {
                    std::string field = pr.substr(0, pos);
                    std::string val = pr.substr(pos+1);
                    hash[field] = val;  
                }
            }
            hash_store[key] = hash;
        }
    }

    return true;
}

bool RedisDatabase::flushAll() {
    std::lock_guard<std::mutex> lock(db_mutex);
    kv_store.clear();
    list_store.clear();
    hash_store.clear();
    return true;

}

// Key value operations
void RedisDatabase::set(const std::string& key, const std::string& val){ 
    std::lock_guard<std::mutex> lock(db_mutex);
    kv_store[key] = val;
}
bool RedisDatabase::get(const std::string& key, std::string& val){ 
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = kv_store.find(key);
    if (it != kv_store.end()) {
        val = it->second;
        return true;
    }
    return false;
}

std::vector<std::string> RedisDatabase::keys(){ 
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<std::string> result;
    for (const auto& pr : kv_store) {
        result.push_back(pr.first);
    }

    for (const auto& pr : list_store) {
        result.push_back(pr.first);
    }

    for (const auto& pr : hash_store) {
        result.push_back(pr.first);
    }
    return result;
}

std::string RedisDatabase::type(const std::string& key){ 
    std::lock_guard<std::mutex> lock(db_mutex);
    if (kv_store.find(key) != kv_store.end()) return "string";
    if (list_store.find(key) != list_store.end()) return "list";
    if (hash_store.find(key) != hash_store.end()) return "hash";
    else return "none";
}

bool RedisDatabase::del(const std::string& key){ 
    std::lock_guard<std::mutex> lock(db_mutex);
    bool erased = false;
    erased |= kv_store.erase(key) > 0;
    erased |= list_store.erase(key) > 0;
    erased |= hash_store.erase(key) > 0;
    return erased;
}

// Expire
bool RedisDatabase::expire(const std::string& key, int sec){ 
    std::lock_guard<std::mutex> lock(db_mutex);
    bool exists = (kv_store.find(key) != kv_store.end()) ||
                 (list_store.find(key) != list_store.end()) || 
                 (hash_store.find(key) != hash_store.end());
    if (!exists) return false;

    expiry_map[key] = std::chrono::steady_clock::now() + std::chrono::seconds(sec);
    return true;
}

// Rename 
bool RedisDatabase::rename(const std::string& oldKey, const std::string& newKey){ 
    std::lock_guard<std::mutex> lock(db_mutex);
    bool found = false;

    auto itKv = kv_store.find(oldKey);
    if (itKv != kv_store.end()) {
        kv_store[newKey] = itKv->second;
        kv_store.erase(itKv);
        found = true;
    }

    auto itList = list_store.find(oldKey);
    if (itList != list_store.end()) {
        list_store[newKey] = itList->second;
        list_store.erase(itList);
        found = true;
    }

    auto itHash = hash_store.find(oldKey);
    if (itHash != hash_store.end()) {
        hash_store[newKey] = itHash->second;
        hash_store.erase(itHash);
        found = true;
    }

    auto itExpire = expiry_map.find(oldKey);
    if (itExpire != expiry_map.end()) {
        expiry_map[newKey] = itExpire->second;
        expiry_map.erase(itExpire);
        found = true;
    }
    return found;
}
