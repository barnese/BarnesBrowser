#include "task_runner.h"

#include <iostream>

TaskRunner::TaskRunner()
{
}

void TaskRunner::scheduleTask(std::unique_ptr<ITask> task)
{
    m_tasks.push_back(std::move(task));
}

void TaskRunner::run()
{
    if (m_tasks.empty())
    {
        return;
    }

    auto task = std::move(m_tasks.back());
    m_tasks.pop_back();

    std::cout << "TaskRunner running: " << task->getName() << std::endl;
    task->run();
}
