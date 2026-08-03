#pragma once
#include "../core/ITask.h"
#include <string>

class RelocateTask : public ITask {
private:
    std::string source_location;
    std::string dest_location;

public:
    RelocateTask(int id, const std::string& name, int priority, 
                 const std::string& source, const std::string& dest);
    bool execute() override;
};