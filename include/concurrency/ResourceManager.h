#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>

class ResourceManager {
private:
    // C++17 inline statics mean we don't need to initialize these in a .cpp file
    inline static std::unordered_map<std::string, std::shared_ptr<std::mutex>> aisle_locks;
    inline static std::mutex map_mutex;

public:
    static std::shared_ptr<std::mutex> getAisleLock(const std::string& location) {
        std::lock_guard<std::mutex> lock(map_mutex);
        if (aisle_locks.find(location) == aisle_locks.end()) {
            aisle_locks[location] = std::make_shared<std::mutex>();
        }
        return aisle_locks[location];
    }
};