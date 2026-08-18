#pragma once

#include "task.h"

#include <condition_variable>
#include <queue>
#include <mutex>
#include <thread>

/// Defines a task runner, which stores a first-in, first-out queue.
class TaskRunner
{
public:
    TaskRunner();
    ~TaskRunner();
    void scheduleTask(std::unique_ptr<ITask> task);
    void setNeedsQuit();
    bool isRunning();
    void clearPendingTasks();

private:
    void run(std::stop_token stopToken);

    std::queue<std::unique_ptr<ITask>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable_any m_condition;

    // Declare the thread last so it is destroyed and 
    // joined before the other members are destroyed.
    std::jthread m_thread;
};
