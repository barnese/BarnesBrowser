#include "task_runner.h"

#include <iostream>

TaskRunner::TaskRunner()
{
}

void TaskRunner::scheduleTask(std::unique_ptr<ITask> task)
{
    // Prevent simultaneous access to the queue by locking a mutex,
    // which will be automatically unlocked when we leave scope.
    std::lock_guard<std::mutex> lock(m_mutex);
    m_tasks.push(std::move(task));
}

void TaskRunner::run()
{
    std::unique_ptr<ITask> task = nullptr;

    {
        std::unique_lock lock(m_mutex);

        if (m_tasks.empty())
        {
            return;
        }

        task = std::move(m_tasks.front());
        m_tasks.pop();
    } // Lock is released here

    // The task is run after unlocking the thread because it has already
    // been released from the queue and can't be accessed by another thread.
    std::cout << "TaskRunner running: " << task->getName() << std::endl;
    task->run();
}
