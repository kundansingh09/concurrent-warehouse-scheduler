#pragma once
#include "../core/ITask.h"
#include <string>

class FetchTask : public ITask {
private:
    std::string item_location;

public:
    FetchTask(int id, const std::string& name, int priority, const std::string& location);
    bool execute() override;
};