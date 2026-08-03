#include "../../include/tasks/FetchTask.h"
#include "../../include/utils/Logger.h"
#include "../../include/concurrency/ResourceManager.h" 
#include <thread>
#include <chrono>
#include <cstdlib>

using namespace std;

FetchTask::FetchTask(int id, const string& name, int priority, const string& location)
    : ITask(id, name, priority), item_location(location) {}

bool FetchTask::execute() {
    // Get the shared physical lock for this aisle
    auto my_aisle_lock = ResourceManager::getAisleLock(item_location);

    Logger::log(Logger::CYAN, "ROBOT", "Task " + to_string(getId()) + " approaching " + item_location + "...");

    // Lock the physical Aisle
    lock_guard<mutex> physical_lock(*my_aisle_lock);
    
    Logger::log(Logger::CYAN, "ROBOT", "Task " + to_string(getId()) + " ENTERED " + item_location);

    this_thread::sleep_for(chrono::milliseconds(500));
    
    if (rand() % 100 < 20) {
        Logger::log(Logger::RED, "ERROR", "Task " + to_string(getId()) + " hardware timeout at " + item_location + "!");
        return false; 
    }

    Logger::log(Logger::GREEN, "DONE", "Successfully fetched Task " + to_string(getId()));
    return true;
}