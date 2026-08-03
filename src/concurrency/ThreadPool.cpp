#include "../../include/concurrency/ThreadPool.h"

using namespace std;

ThreadPool::ThreadPool(size_t num_threads) : stop(false) {
    // Spawn the worker threads
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this] { this->workerLoop(); });
    }
}

ThreadPool::~ThreadPool() {
    {
        lock_guard<mutex> lock(queue_mutex);
        stop = true;
    }
    
    // Wake up ALL sleeping threads
    condition.notify_all();
    
    // Wait for threads to finish their current task and terminate
    for (thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::enqueue(shared_ptr<ITask> task, function<void(shared_ptr<ITask>, bool)> callback) {
    {
        // Lock the queue
        lock_guard<mutex> lock(queue_mutex);
        task_queue.push({task, callback});
    }
    
    // Wake up exactly ONE sleeping thread
    condition.notify_one();
}

void ThreadPool::workerLoop() {
    while (true) {
        TaskPair current_job;
        
        {
            unique_lock<mutex> lock(queue_mutex);
            
            condition.wait(lock, [this] { 
                return stop || !task_queue.empty(); 
            });
            
            if (stop && task_queue.empty()) {
                return;
            }
            
            current_job = task_queue.top();
            task_queue.pop();
        } 

        // Execute the task outside the lock 
        current_job.first->setState(TaskState::RUNNING);
        bool success = current_job.first->execute();
        
        current_job.second(current_job.first, success);
    }
}