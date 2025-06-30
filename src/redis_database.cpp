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

// List ops
std::vector<std::string> RedisDatabase::lget(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it != list_store.end()) return it->second; 

    return {}; 
}

ssize_t RedisDatabase::llen(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it != list_store.end()) 
        return it->second.size();
    return 0;
}

void RedisDatabase::lpush(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    list_store[key].insert(list_store[key].begin(), value);
}

void RedisDatabase::rpush(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    list_store[key].push_back(value);
}

bool RedisDatabase::lpop(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it != list_store.end() && !it->second.empty()) {
        value = it->second.front();
        it->second.erase(it->second.begin());
        return true;
    }
    return false;
}

bool RedisDatabase::rpop(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it != list_store.end() && !it->second.empty()) {
        value = it->second.back();
        it->second.pop_back();
        return true;
    }
    return false;
}

// If count is positive, remove from start, else remove from left
int RedisDatabase::lrem(const std::string& key, int count, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    int removed = 0;
    auto it = list_store.find(key);
    if (it == list_store.end()) 
        return 0;

    auto& lst = it->second;

    if (count == 0) {
        // Remove all, remove() pushes unwanted elements to the back
        auto new_end = std::remove(lst.begin(), lst.end(), value);

        // Distance() returns no. of hops btw the two iterators
        removed = std::distance(new_end, lst.end());

        // Remove unwanted elements
        lst.erase(new_end, lst.end());
    } else if (count > 0) {
        // Remove from start
        for (auto it = lst.begin(); it != lst.end() && removed < count; ) {
            if (*it == value) {
                it = lst.erase(it);
                ++removed;
            } else {
                ++it;
            }
        }
    } else {
        // Remove from end, make count positive
        for (auto reverseIt = lst.rbegin(); reverseIt != lst.rend() && removed < (-count); ) {
            if (*reverseIt == value) {
                // Base() returns a fwd iter pointing to the element after the current elem
                auto fwdIter = reverseIt.base();
                --fwdIter;

                fwdIter = lst.erase(fwdIter);
                ++removed;

                // Convert back to reverse it
                reverseIt = std::reverse_iterator<std::vector<std::string>::iterator>(fwdIter);
            } else {
                ++reverseIt;
            }
        }
    }
    return removed;
}

// Retrieve corresponding item in the selected list using index
bool RedisDatabase::lindex(const std::string& key, int index, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);

    // If list doesnt exists
    if (it == list_store.end()) return false;

    const auto& lst = it->second;

    // If index is negative, its using the index starting from the end of the list
    if (index < 0) index = lst.size() + index;

    // Out of bounds
    if (index < 0 || index >= static_cast<int>(lst.size()))return false;
    
    value = lst[index];
    return true;
}

bool RedisDatabase::lset(const std::string& key, int index, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it == list_store.end()) return false;

    auto& lst = it->second;
    if (index < 0) index = lst.size() + index;
    if (index < 0 || index >= static_cast<int>(lst.size())) return false;
    
    lst[index] = value;
    return true;
}

// Hash Ops
bool RedisDatabase::hset(const std::string& key, const std::string& field, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    hash_store[key][field] = value;
    return true;
}

bool RedisDatabase::hget(const std::string& key, const std::string& field, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);

    // Find by key then by field
    auto it = hash_store.find(key);
    if (it != hash_store.end()) {
        auto fieldIt = it->second.find(field);
        if (fieldIt != it->second.end()) {
            value = fieldIt->second;
            return true;
        }
    }
    return false;
}

bool RedisDatabase::hexists(const std::string& key, const std::string& field) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = hash_store.find(key);
    if (it != hash_store.end()) return it->second.find(field) != it->second.end();
    return false;
}

bool RedisDatabase::hdel(const std::string& key, const std::string& field) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = hash_store.find(key);
    if (it != hash_store.end()) return it->second.erase(field) > 0;
    return false;
}

std::unordered_map<std::string, std::string> RedisDatabase::hgetall(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (hash_store.find(key) != hash_store.end()) return hash_store[key];
    return {};
}

std::vector<std::string> RedisDatabase::hkeys(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);

    // push a copy of the fields
    std::vector<std::string> fields;
    auto it = hash_store.find(key);
    if (it != hash_store.end()) {
        for (const auto& pair: it->second)
            fields.push_back(pair.first);
    }
    return fields;
}

std::vector<std::string> RedisDatabase::hvals(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<std::string> values;
    auto it = hash_store.find(key);
    if (it != hash_store.end()) {
        for (const auto& pair: it->second)
            values.push_back(pair.second);
    }
    return values;
}

ssize_t RedisDatabase::hlen(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = hash_store.find(key);
    return (it != hash_store.end()) ? it->second.size() : 0;
}

bool RedisDatabase::hmset(const std::string& key, const std::vector<std::pair<std::string, std::string>>& fieldValues) {
    std::lock_guard<std::mutex> lock(db_mutex);
    for (const auto& pair: fieldValues) {
        hash_store[key][pair.first] = pair.second;
    }
    return true;
}
