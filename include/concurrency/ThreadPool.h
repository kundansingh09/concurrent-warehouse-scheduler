#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include "../core/ITask.h"

class ThreadPool {
private:
    std::vector<std::thread> workers;
    
    using TaskPair = std::pair<std::shared_ptr<ITask>, std::function<void(std::shared_ptr<ITask>, bool)>>;
    
    // Custom comparator for the priority queue
    struct CompareTaskPriority {
        bool operator()(const TaskPair& a, const TaskPair& b) {
            return a.first->getPriority() < b.first->getPriority();
        }
    };

    std::priority_queue<TaskPair, std::vector<TaskPair>, CompareTaskPriority> task_queue;

    // Synchronization primitives
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;

    // The infinite loop that each worker thread runs
    void workerLoop();

public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    void enqueue(std::shared_ptr<ITask> task, std::function<void(std::shared_ptr<ITask>, bool)> callback);
};