cmake_minimum_required(VERSION 3.10)
project(WarehouseScheduler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Enterprise ThreadSanitizer flags to catch data races automatically
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g")

# Gate TSan behind an option so it doesn't slow down standard builds
option(ENABLE_TSAN "Enable ThreadSanitizer for data race detection" OFF)
if(ENABLE_TSAN)
    message(STATUS "ThreadSanitizer ENABLED")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
endif()

# 1. Main Application Executable
add_executable(warehouse_scheduler 
    src/main.cpp
    src/graph/TaskGraph.cpp
    src/concurrency/ThreadPool.cpp
    src/concurrency/Scheduler.cpp
    src/tasks/FetchTask.cpp
    src/tasks/PackTask.cpp
    src/tasks/RelocateTask.cpp
)
target_link_libraries(warehouse_scheduler Threads::Threads)

# 2. Unit Test Executable
add_executable(run_unit_tests 
    tests/unit_tests.cpp
    src/graph/TaskGraph.cpp
    src/concurrency/ThreadPool.cpp
    src/concurrency/Scheduler.cpp
    src/tasks/FetchTask.cpp
    src/tasks/PackTask.cpp
    src/tasks/RelocateTask.cpp
)
target_link_libraries(run_unit_tests Threads::Threads)

# 3. Integration Test Executable
add_executable(run_integration_tests 
    tests/integration_tests.cpp
    src/graph/TaskGraph.cpp
    src/concurrency/ThreadPool.cpp
    src/concurrency/Scheduler.cpp
    src/tasks/FetchTask.cpp
    src/tasks/PackTask.cpp
    src/tasks/RelocateTask.cpp
)
target_link_libraries(run_integration_tests Threads::Threads)

# 4. Stress Test Executable
add_executable(run_stress_tests 
    tests/stress_tests.cpp
    src/graph/TaskGraph.cpp
    src/concurrency/ThreadPool.cpp
    src/concurrency/Scheduler.cpp
    src/tasks/FetchTask.cpp
    src/tasks/PackTask.cpp
    src/tasks/RelocateTask.cpp
)
target_link_libraries(run_stress_tests Threads::Threads)