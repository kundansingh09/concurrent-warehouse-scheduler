#pragma once
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include "../core/ITask.h"

class TaskGraph {
private:
    std::unordered_map<int, std::shared_ptr<ITask>> tasks;
    std::unordered_map<int, std::vector<int>> adj_list;
    std::unordered_map<int, int> in_degree;
    std::mutex graph_mutex;

public:
    TaskGraph() = default;
    ~TaskGraph() = default;

    void addTask(std::shared_ptr<ITask> task);
    void addDependency(int from_id, int to_id);
    
    // Cycle detection using Kahn's Algorithm
    bool hasCycles();
    
    // Runtime dependency resolution (Dynamic Kahn's)
    std::vector<std::shared_ptr<ITask>> getIndependentTasks();
    std::vector<std::shared_ptr<ITask>> markTaskCompleted(int task_id);
    
    std::shared_ptr<ITask> getTask(int task_id);

    int cancelDependentTasks(int failed_task_id);
};