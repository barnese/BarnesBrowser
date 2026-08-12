#pragma once

#include "task.h"

/// Defines a task runner, which stores a first-in, first-out queue.
class TaskRunner
{
public:
    TaskRunner();
    void scheduleTask(std::unique_ptr<ITask> task);
    void run();

private:
    std::vector<std::unique_ptr<ITask>> m_tasks;
};
