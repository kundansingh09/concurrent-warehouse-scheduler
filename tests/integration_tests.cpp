#include <iostream>
#include <memory>
#include <cassert>
#include "../include/graph/TaskGraph.h"
#include "../include/tasks/FetchTask.h"
#include "../include/tasks/PackTask.h"
#include "../include/concurrency/ThreadPool.h"
#include "../include/concurrency/Scheduler.h"
#include "../include/tasks/RelocateTask.h"

using namespace std;

shared_ptr<FetchTask> makeDummyTask(int id, int priority = 1) {
    return make_shared<FetchTask>(id, "TestTask", priority, "Test Aisle");
}

int main() {
    cout << "\n=================================================\n";
    cout << "       FULL SYSTEM INTEGRATION TEST              \n";
    cout << "=================================================\n";
    
    TaskGraph graph;
    
    // Order 1
    auto fetch1 = makeDummyTask(1, 1); 
    auto fetch2 = makeDummyTask(2, 1); 
    auto pack1 = make_shared<PackTask>(3, "Pack Order 1", 2, "ORD-1");
    graph.addTask(fetch1); graph.addTask(fetch2); graph.addTask(pack1);
    graph.addDependency(1, 3); graph.addDependency(2, 3);

    // Order 2 (Competes for resources)
    auto fetch3 = makeDummyTask(4, 1);
    auto pack2 = make_shared<PackTask>(5, "Pack Order 2", 2, "ORD-2");
    graph.addTask(fetch3); graph.addTask(pack2);
    graph.addDependency(4, 5);
    
    // Dispatch
    auto ship = make_shared<FetchTask>(6, "Ship All", 3, "Loading Bay");
    graph.addTask(ship);
    graph.addDependency(3, 6);
    graph.addDependency(5, 6);

    // Run the actual engine with 2 threads
    ThreadPool pool(2);
    Scheduler scheduler(graph, pool, 7);
    
    auto relocate = make_shared<RelocateTask>(7, "Move Heavy Pallet", 1, "Aisle 1", "Aisle 2"); 
    graph.addTask(relocate);

    graph.addDependency(relocate->getId(), 3);

    bool success = scheduler.start();
    
    assert(success == true && "Scheduler should successfully complete all tasks without hanging.");
    cout << "  -> Engine successfully resolved all concurrent dependencies.\n";
    cout << "=================================================\n\n";
    
    return 0;
}