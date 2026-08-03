# Concurrent Warehouse Task Scheduler

![C++17](https://img.shields.io/badge/C++-17-blue.svg) ![CMake](https://img.shields.io/badge/Concurrency-ThreadPool-orange.svg) ![License](https://img.shields.io/badge/License-MIT-purple.svg)

A high-performance, multithreaded task execution engine simulating a modern distributed Warehouse Management System (WMS). Engineered to handle highly concurrent robot routing, resource contention, and fault-tolerant DAG (Directed Acyclic Graph) resolution.

---

## Architecture & Core Features

This engine is built on enterprise-grade OS and Concurrency concepts, designed to process overlapping orders safely and efficiently.

### 1. Topological Graph Resolution (Logical Deadlock Prevention)
* **Kahn's Algorithm:** Tasks are modeled as a Directed Acyclic Graph (DAG). Before execution begins, the system mathematically verifies that no cyclic dependencies exist (e.g., Task A waiting on Task B, while Task B waits on Task A). If a cycle is detected, the boot sequence aborts to prevent a system-wide freeze.
* **O(Children) Unlocking:** Upon task completion, only direct child dependencies are evaluated, bypassing heavy O(N) graph rescans.

### 2. Custom ThreadPool & Priority Preemption
* Replaces standard FIFO queues with a **Priority-Based Preemption Queue** (`std::priority_queue`). 
* High-priority shipping tasks immediately preempt standard fetch tasks across a highly concurrent pool of OS-level worker threads.

### 3. Resource Contention & Physical Deadlock Avoidance
* **Centralized ResourceManager:** Individual aisles and loading bays are protected by striped mutex locks.
* **Circular Wait Prevention:** Multi-resource acquisitions (e.g., a Forklift needing Aisle 1 and Aisle 2 simultaneously to relocate a pallet) are secured atomically using C++11's `std::lock` and `std::adopt_lock` algorithms, mathematically preventing circular wait deadlocks.

### 4. Extreme Fault Tolerance (Cascading Failure Prevention)
* Hardware timeouts are simulated and caught via an automated retry mechanism.
* **BFS Cascading Cancellation:** If a task permanently fails after exhausting max retries, a Breadth-First Search traverses the graph to selectively cancel all dependent child tasks, preventing memory leaks and infinite server hangs.

---

## How It Takes Input

Currently, the scheduler operates as a standalone core execution engine. Input is provided in two ways:
1. **Interactive CLI (Main App):** Users can dynamically generate grouped orders (Fetch -> Pack -> Ship) directly in the terminal.
2. **Deterministic Scripts (Integration Tests):** Highly specific, overlapping multithreaded scenarios are hardcoded in the test suite to verify physical resource contention (e.g., forcing two robots into the same aisle).

### Future Expansion: Event-Driven Inputs
To scale this into a full microservices backend, the input layer can be decoupled:
* **Message Brokers:** The `TaskGraph` could consume `inventory_reserved` events directly from an Apache Kafka or RabbitMQ queue.
* **API Gateway:** A Node.js/Python layer would receive JSON payloads from a frontend, validate stock in an external database, and asynchronously push the compiled dependency graph to this C++ execution engine.

---

## How to Read Outputs

The system utilizes a custom, inline-static mutex Logger to guarantee **Thread-Safe Console Output**. This prevents garbled text when 16 threads print simultaneously.

The logs are ANSI color-coded for instant dashboard readability:
* **CYAN `[ROBOT]`:** Standard operations (Robot approaching/entering an aisle).
* **YELLOW `[FORKLIFT]` / `[RETRY]`:** Multi-resource acquisition or fault-tolerance warnings (task failure, automatically re-queuing).
* **RED `[FATAL]` / `[CASCADE]`:** Permanent hardware failures and the subsequent graph-pruning operations.
* **GREEN `[DONE]`:** Successful task completion and inventory sealing.

*(In a production environment, these logs would be swapped for a Webhook or Observer Pattern, pushing real-time `[TASK_COMPLETED]` state changes back to a frontend user dashboard.)*

---

## Build & Run Instructions

This project uses **CMake** for cross-platform building and has been audited against data races using **ThreadSanitizer (TSan)**.

### 1. Build the Project
```bash
mkdir -p build && cd build
cmake ..
make
```
*(To build with Enterprise Data Race detection enabled, use: `cmake -DENABLE_TSAN=ON ..`)*

### 2. Run the Engine Tiers
We utilize a structured Testing Pyramid. Run these directly from your `build/` directory:

* **Interactive Application:**
  ```bash
  ./warehouse_scheduler
  ```
* **Unit Tests (Fast Logic Checks):**
  ```bash
  ./run_unit_tests
  ```
* **Integration Tests (Multithreaded Workflow Verification):**
  ```bash
  ./run_integration_tests
  ```
* **Extreme Chaos Stress Test (10,000 Tasks):**
  ```bash
  ./run_stress_tests
  ```

---

## License

This project is open-source and available under the [MIT License](LICENSE).
