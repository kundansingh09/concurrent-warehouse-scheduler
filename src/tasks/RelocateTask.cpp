#include "../../include/tasks/RelocateTask.h"
#include "../../include/utils/Logger.h"
#include "../../include/concurrency/ResourceManager.h"
#include <thread>
#include <chrono>
#include <mutex>

using namespace std;

RelocateTask::RelocateTask(int id, const string& name, int priority, 
                           const string& source, const string& dest)
    : ITask(id, name, priority), source_location(source), dest_location(dest) {}

bool RelocateTask::execute() {
    auto lock1 = ResourceManager::getAisleLock(source_location);
    auto lock2 = ResourceManager::getAisleLock(dest_location);

    Logger::log(Logger::YELLOW, "FORKLIFT", "Task " + to_string(getId()) + 
                " requesting move from " + source_location + " to " + dest_location);

    if (source_location == dest_location) {
        // EDGE CASE: Prevent UB and self-deadlock by only locking once
        lock_guard<mutex> lg(*lock1);
        Logger::log(Logger::YELLOW, "FORKLIFT", "Task " + to_string(getId()) + 
                    " SECURED " + source_location + " (Internal Move). Moving inventory...");
        this_thread::sleep_for(chrono::milliseconds(1000));
    } else {
        std::lock(*lock1, *lock2);
        lock_guard<mutex> lg1(*lock1, std::adopt_lock);
        lock_guard<mutex> lg2(*lock2, std::adopt_lock);
        
        Logger::log(Logger::YELLOW, "FORKLIFT", "Task " + to_string(getId()) + 
                    " SECURED BOTH AISLES. Moving inventory...");
        this_thread::sleep_for(chrono::milliseconds(1000));
    }

    Logger::log(Logger::GREEN, "DONE", "Successfully relocated inventory (Task " + to_string(getId()) + ")");
    return true;
}