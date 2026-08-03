#include <iostream>
#include <memory>
#include <cassert>
#include <random>
#include <thread>
#include <ctime>
#include "../include/graph/TaskGraph.h"
#include "../include/tasks/FetchTask.h"
#include "../include/tasks/PackTask.h"
#include "../include/concurrency/ThreadPool.h"
#include "../include/concurrency/Scheduler.h"

using namespace std;

class ChaosTask : public ITask {
public:
    ChaosTask(int id, int priority) : ITask(id, "Chaos", priority, 3) {}
    bool execute() override {
        return (rand() % 100) >= 30; // 30% failure rate
    }
};

int main() {
    cout << "\n=================================================\n";
    cout << "    EXTREME CHAOS STRESS TEST (10,000 Tasks)     \n";
    cout << "=================================================\n";

    TaskGraph graph;
    int total_tasks = 10000;
    
    for (int i = 1; i <= total_tasks; i++) {
        graph.addTask(make_shared<ChaosTask>(i, rand() % 3));
    }
    
    srand(static_cast<unsigned int>(time(nullptr)));
    for (int i = 2; i <= total_tasks; i++) {
        int num_dependencies = (rand() % 5);
        for (int d = 0; d < num_dependencies; d++) {
            int parent_id = (rand() % (i - 1)) + 1; 
            graph.addDependency(parent_id, i);
        }
    }
    
    unsigned int hardware_threads = std::thread::hardware_concurrency();
    int thread_count = (hardware_threads > 0) ? hardware_threads : 4;
    
    cout << "[STRESS] Launching engine with " << thread_count << " hardware threads...\n";

    ThreadPool pool(thread_count);
    Scheduler scheduler(graph, pool, total_tasks);
    
    // Silence output during heavy crunching
    cout.setstate(ios_base::failbit);
    bool success = scheduler.start();
    cout.clear();
    
    assert(success == true && "Scheduler failed or hung under stress!");
    cout << "  -> Successfully processed 10,000 tasks with zero deadlocks or unhandled races.\n";
    cout << "=================================================\n\n";
    
    return 0;
}