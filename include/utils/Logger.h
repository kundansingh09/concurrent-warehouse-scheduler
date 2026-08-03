#pragma once
#include <iostream>
#include <mutex>
#include <string>

class Logger {
private:
    // A single mutex shared across the entire program to protect std::cout
    inline static std::mutex log_mutex;

public:
    // ANSI Color Codes
    inline static const std::string RESET   = "\033[0m";
    inline static const std::string RED     = "\033[31m";
    inline static const std::string GREEN   = "\033[32m";
    inline static const std::string YELLOW  = "\033[33m";
    inline static const std::string CYAN    = "\033[36m";
    inline static const std::string MAGENTA = "\033[35m";

    static void log(const std::string& color, const std::string& tag, const std::string& message) {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cout << color << "[" << tag << "] " << RESET << message << "\n";
    }
};