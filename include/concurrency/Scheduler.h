#pragma once
#include <memory>
#include <mutex>
#include <condition_variable>
#include "../graph/TaskGraph.h"
#include "ThreadPool.h"

class Scheduler {
private:
    TaskGraph& graph;
    ThreadPool& pool;

    int total_tasks;
    int processed_tasks; 
    std::mutex sched_mutex;
    std::condition_variable sched_cv;

    // Pulls ready tasks from the graph and pushes them to the thread pool
    void scheduleReadyTasks();

    void onTaskFinished(std::shared_ptr<ITask> task, bool success);

public:
    Scheduler(TaskGraph& graph, ThreadPool& pool, int total_tasks);
    ~Scheduler() = default;

    // Kicks off the entire warehouse execution
    bool start(); 
};