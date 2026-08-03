#pragma once
#include <string>
#include <atomic>
#include "TaskState.h"

// used std:: explicitly in headers to avoid namespace pollution
class ITask {
protected:
    int id;
    std::string name;
    int priority; 
    std::atomic<TaskState> state;
    int max_retries;
    int current_retries;

public:
    // constructor 
    ITask(int id, const std::string& name, int priority, int retries = 3) 
        : id(id), name(name), priority(priority), 
          state(TaskState::PENDING), max_retries(retries), current_retries(0) {}

    virtual ~ITask() = default;

    // Pure virtual function
    virtual bool execute() = 0;

    // Getters
    int getId() const { return id; }
    std::string getName() const { return name; }
    int getPriority() const { return priority; }
    TaskState getState() const { return state.load(); }
    int getCurrentRetries() const { return current_retries; }
    int getMaxRetries() const { return max_retries; }
    
    // Setters
    void setState(TaskState new_state) { state.store(new_state); }
    
    // Fault tolerance helpers
    bool canRetry() const { return current_retries < max_retries; }
    void incrementRetry() { current_retries++; }
    void resetForRetry() { state.store(TaskState::READY); }
};