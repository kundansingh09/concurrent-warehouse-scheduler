#include "../../include/tasks/PackTask.h"
#include "../../include/utils/Logger.h"
#include <thread>
#include <chrono>

using namespace std;

PackTask::PackTask(int id, const string& name, int priority, const string& order_id)
    : ITask(id, name, priority), order_id(order_id) {}

bool PackTask::execute() {
    Logger::log(Logger::CYAN, "PACKER", "Boxing Task " + to_string(getId()) + " for " + order_id);
    
    this_thread::sleep_for(chrono::milliseconds(800));
    
    Logger::log(Logger::GREEN, "DONE", "Successfully sealed " + order_id + " (Task " + to_string(getId()) + ")");
    return true; 
}