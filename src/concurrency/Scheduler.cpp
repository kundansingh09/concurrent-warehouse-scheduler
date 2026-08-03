#include "../../include/concurrency/Scheduler.h"
#include "../../include/utils/Logger.h"

using namespace std;

Scheduler::Scheduler(TaskGraph& g, ThreadPool& p, int total) 
    : graph(g), pool(p), total_tasks(total), processed_tasks(0) {}

bool Scheduler::start() {
    Logger::log(Logger::MAGENTA, "SYSTEM", "Booting up Concurrent Task Scheduler...");
    Logger::log(Logger::MAGENTA, "SYSTEM", "Running Graph Cycle Detection (Deadlock Prevention)...");
    
    if (graph.hasCycles()) {
        Logger::log(Logger::RED, "FATAL", "Cycle detected in task dependencies. Aborting to prevent deadlock.");
        return false;
    }
    Logger::log(Logger::MAGENTA, "SYSTEM", "Graph is acyclic and safe. Starting execution.");

    // Initial injection of tasks (Tasks with 0 dependencies)
    scheduleReadyTasks();

    // Put the main thread to sleep until all tasks are processed
    unique_lock<mutex> lock(sched_mutex);
    sched_cv.wait(lock, [this] { 
        return processed_tasks >= total_tasks; 
    });

    Logger::log(Logger::MAGENTA, "SYSTEM", "All tasks processed. Warehouse shutdown complete.");
    return true;
}

void Scheduler::scheduleReadyTasks() {
    // Kahn's algorithm: Get tasks where in-degree == 0
    auto ready_tasks = graph.getIndependentTasks();
    
    for (auto& task : ready_tasks) {
        pool.enqueue(task, [this](shared_ptr<ITask> t, bool s) {
            this->onTaskFinished(t, s);
        });
    }
}

void Scheduler::onTaskFinished(shared_ptr<ITask> task, bool success) {
    if (success) {
        auto unlocked_tasks = graph.markTaskCompleted(task->getId());
        
        {
            lock_guard<mutex> lock(sched_mutex);
            processed_tasks++;
            sched_cv.notify_one();
        }
        
        // Queue them
        for (auto& next_task : unlocked_tasks) {
            pool.enqueue(next_task, [this](shared_ptr<ITask> t, bool s) {
                this->onTaskFinished(t, s);
            });
        }
    } else {
        if (task->canRetry()) {
            task->incrementRetry();
            
            // Thread-safe warning log for the retry mechanism
            Logger::log(Logger::YELLOW, "RETRY", "Task " + to_string(task->getId()) + 
                        " failed. Re-queuing (Attempt " + to_string(task->getCurrentRetries()) + 
                        " of " + to_string(task->getMaxRetries()) + ").");
                 
            task->resetForRetry();
            
            pool.enqueue(task, [this](shared_ptr<ITask> t, bool s) {
                this->onTaskFinished(t, s);
            });
        } else {
            Logger::log(Logger::RED, "FATAL", "Task " + to_string(task->getId()) + 
                        " permanently failed after max retries.");
            task->setState(TaskState::FAILED);
            
            // Execute Cascading Failure Prevention
            int cancelled_children = graph.cancelDependentTasks(task->getId());
            
            if (cancelled_children > 0) {
                Logger::log(Logger::RED, "CASCADE", "Cancelled " + to_string(cancelled_children) + 
                            " dependent tasks due to Task " + to_string(task->getId()) + " failure.");
            }
            
            {
                lock_guard<mutex> lock(sched_mutex);
                // We add 1 for the failed task itself, PLUS all the children we just killed
                processed_tasks += (1 + cancelled_children); 
                sched_cv.notify_one();
            }
        }
    }
}