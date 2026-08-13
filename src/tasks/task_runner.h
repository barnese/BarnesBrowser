#pragma once

#include "task.h"

#include <queue>
#include <mutex>

/// Defines a task runner, which stores a first-in, first-out queue.
class TaskRunner
{
public:
    TaskRunner();
    void scheduleTask(std::unique_ptr<ITask> task);
    void run();

private:
    std::queue<std::unique_ptr<ITask>> m_tasks;
    std::mutex m_mutex;
};
