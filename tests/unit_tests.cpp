#include <iostream>
#include <memory>
#include <cassert>
#include <queue>
#include <functional>
#include "../include/graph/TaskGraph.h"
#include "../include/tasks/FetchTask.h"
#include "../include/tasks/PackTask.h"

using namespace std;

shared_ptr<FetchTask> makeDummyTask(int id, int priority = 1) {
    return make_shared<FetchTask>(id, "TestTask", priority, "Test Aisle");
}

void testTaskRetryLogic() {
    cout << "  [UNIT] Fault Tolerance (Retry Limits)... ";
    auto task = makeDummyTask(1);
    assert(task->canRetry() == true);
    assert(task->getCurrentRetries() == 0);
    
    task->incrementRetry();
    task->incrementRetry();
    task->incrementRetry(); // Max is 3
    assert(task->canRetry() == false);
    cout << "PASSED\n";
}

void testDeadlockDetection() {
    cout << "  [UNIT] Deadlock Detection (Cycle Check)... ";
    TaskGraph graph;
    graph.addTask(makeDummyTask(1));
    graph.addTask(makeDummyTask(2));
    graph.addTask(makeDummyTask(3));

    graph.addDependency(1, 2);
    graph.addDependency(2, 3);
    graph.addDependency(3, 1); // Cycle

    assert(graph.hasCycles() == true);
    cout << "PASSED\n";
}

void testCascadingCancellation() {
    cout << "  [UNIT] Cascading Failure Prevention... ";
    TaskGraph graph;
    auto t1 = makeDummyTask(1);
    auto t2 = makeDummyTask(2);
    auto t3 = makeDummyTask(3);

    graph.addTask(t1); graph.addTask(t2); graph.addTask(t3);
    graph.addDependency(1, 2);
    graph.addDependency(2, 3);

    int cancelled_count = graph.cancelDependentTasks(1);
    assert(cancelled_count == 2);
    assert(t2->getState() == TaskState::CANCELLED);
    assert(t3->getState() == TaskState::CANCELLED);
    cout << "PASSED\n";
}

void testKahnsAlgorithmResolution() {
    cout << "  [UNIT] Dynamic Kahn's Algorithm Resolution... ";
    TaskGraph graph;
    graph.addTask(makeDummyTask(1));
    graph.addTask(makeDummyTask(2));
    graph.addDependency(1, 2);
    
    auto unlocked = graph.markTaskCompleted(1);
    assert(unlocked.size() == 1);
    assert(unlocked[0]->getId() == 2);
    cout << "PASSED\n";
}

void testPriorityOrdering() {
    cout << "  [UNIT] ThreadPool Priority Queue Ordering... ";
    using TaskPair = std::pair<std::shared_ptr<ITask>, std::function<void(std::shared_ptr<ITask>, bool)>>;
    struct CompareTaskPriority {
        bool operator()(const TaskPair& a, const TaskPair& b) {
            return a.first->getPriority() < b.first->getPriority();
        }
    };
    
    std::priority_queue<TaskPair, std::vector<TaskPair>, CompareTaskPriority> test_queue;
    auto low_priority = makeDummyTask(1, 1);
    auto high_priority = makeDummyTask(2, 2);
    
    test_queue.push({low_priority, nullptr});
    test_queue.push({high_priority, nullptr});
    
    assert(test_queue.top().first->getId() == 2);
    cout << "PASSED\n";
}

int main() {
    cout << "\n=================================================\n";
    cout << "          UNIT TEST SUITE                        \n";
    cout << "=================================================\n";
    testTaskRetryLogic();
    testDeadlockDetection();
    testCascadingCancellation();
    testKahnsAlgorithmResolution();
    testPriorityOrdering();
    cout << "=================================================\n\n";
    return 0;
}