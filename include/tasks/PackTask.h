#pragma once
#include "../core/ITask.h"
#include <string>

class PackTask : public ITask {
private:
    std::string order_id;

public:
    PackTask(int id, const std::string& name, int priority, const std::string& order_id);
    bool execute() override;
};