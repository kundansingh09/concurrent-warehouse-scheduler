#pragma once

enum class TaskState {
    PENDING,    // Waiting for dependencies to clear
    READY,      // Dependencies cleared, waiting in queue
    RUNNING,    // Picked up by a worker thread
    COMPLETED,  // Successfully finished
    FAILED,     // Failed (can be retried)
    CANCELLED   // Parent task permanently failed
};