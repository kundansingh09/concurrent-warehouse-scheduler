#include "../../include/graph/TaskGraph.h"
#include <queue>
#include <iostream>

using namespace std;

void TaskGraph::addTask(shared_ptr<ITask> task) {
    lock_guard<mutex> lock(graph_mutex);
    int id = task->getId();
    tasks[id] = task;
    if (in_degree.find(id) == in_degree.end()) {
        in_degree[id] = 0;
    }
}

void TaskGraph::addDependency(int from_id, int to_id) {
    lock_guard<mutex> lock(graph_mutex);
    adj_list[from_id].push_back(to_id);
    in_degree[to_id]++;
}

// Deadlock Detection using Kahn's Algorithm
bool TaskGraph::hasCycles() {
    lock_guard<mutex> lock(graph_mutex);
    
    // Create a temporary copy of in-degrees so we don't ruin the real graph
    unordered_map<int, int> temp_in_degree = in_degree;
    queue<int> zero_degree_queue;
    int processed_nodes = 0;

    // Find all tasks that have 0 dependencies
    for (auto const& [id, degree] : temp_in_degree) {
        if (degree == 0) {
            zero_degree_queue.push(id);
        }
    }

    while (!zero_degree_queue.empty()) {
        int current = zero_degree_queue.front();
        zero_degree_queue.pop();
        processed_nodes++;

        // "Remove" this node by reducing the in-degree of its children
        for (int dependent : adj_list[current]) {
            temp_in_degree[dependent]--;
            if (temp_in_degree[dependent] == 0) {
                zero_degree_queue.push(dependent);
            }
        }
    }

    // 4. If we couldn't process every node, there is a cycle (deadlock)
    return static_cast<size_t>(processed_nodes) != tasks.size();
}

vector<shared_ptr<ITask>> TaskGraph::getIndependentTasks() {
    lock_guard<mutex> lock(graph_mutex);
    vector<shared_ptr<ITask>> ready_tasks;

    for (auto const& [id, degree] : in_degree) {
        if (degree == 0) {
            auto task = tasks[id];
            if (task->getState() == TaskState::PENDING) {
                task->setState(TaskState::READY);
                ready_tasks.push_back(task);
            }
        }
    }
    return ready_tasks;
}

vector<shared_ptr<ITask>> TaskGraph::markTaskCompleted(int task_id) {
    lock_guard<mutex> lock(graph_mutex);
    vector<shared_ptr<ITask>> newly_ready_tasks;
    
    if (tasks.find(task_id) != tasks.end()) {
        tasks[task_id]->setState(TaskState::COMPLETED);
        
        // O(Children) check instead of O(N) full rescan
        for (int dependent_id : adj_list[task_id]) {
            in_degree[dependent_id]--;
            
            if (in_degree[dependent_id] == 0) {
                auto next_task = tasks[dependent_id];
                next_task->setState(TaskState::READY);
                newly_ready_tasks.push_back(next_task);
            }
        }
    }
    return newly_ready_tasks;
}

shared_ptr<ITask> TaskGraph::getTask(int task_id) {
    lock_guard<mutex> lock(graph_mutex);
    if (tasks.find(task_id) != tasks.end()) {
        return tasks[task_id];
    }
    return nullptr;
}

int TaskGraph::cancelDependentTasks(int failed_task_id) {
    lock_guard<mutex> lock(graph_mutex);
    int cancelled_count = 0;
    queue<int> q;
    
    // Start the BFS from the children of the failed task
    for (int child_id : adj_list[failed_task_id]) {
        q.push(child_id);
    }

    while (!q.empty()) {
        int current = q.front();
        q.pop();

        auto task = tasks[current];
        if (task->getState() != TaskState::CANCELLED) {
            task->setState(TaskState::CANCELLED);
            cancelled_count++;
            
            // Continue down the tree
            for (int child_id : adj_list[current]) {
                q.push(child_id);
            }
        }
    }
    
    return cancelled_count;
}