#include "task_runner.h"

#include "helpers/thread_registry.h"

TaskRunner::TaskRunner()
    : m_logger(Logger("TaskRunner")),
      m_thread([this](std::stop_token stopToken) {
            run(stopToken);
        })
{
}

void TaskRunner::scheduleTask(std::unique_ptr<ITask> task)
{
    // Prevent simultaneous access to the queue by locking a mutex,
    // which will be automatically unlocked when we leave scope.
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.push(std::move(task));
    }
    m_condition.notify_one();
}

void TaskRunner::run(std::stop_token stopToken)
{
    // Add the current thread to the thread registry,
    // which allows us to have human-readable thread names.
    threadRegistry().add(std::this_thread::get_id(), "Main Thread");

    m_logger.info("starting");

    while (true)
    {
        std::unique_ptr<ITask> task = nullptr;

        {
            std::unique_lock lock(m_mutex);

            m_condition.wait(
                lock, 
                stopToken,
                [this] { return !m_tasks.empty(); }
            );

            if (stopToken.stop_requested() && m_tasks.empty())
            {
                return;
            }

            task = std::move(m_tasks.front());
            m_tasks.pop();
        } // Lock released here

        // The task is run after unlocking the thread because it has already
        // been released from the queue and can't be accessed by another thread.
        if (!task->getName().empty())        
        {
            m_logger.info("running " + task->getName());
        }
        task->run();
    }
}

bool TaskRunner::isRunning()
{
    return m_thread.joinable();
}

void TaskRunner::setNeedsQuit()
{
    if (!isRunning())
        return;

    m_thread.request_stop();
}

void TaskRunner::clearPendingTasks()
{
    std::unique_lock lock(m_mutex);
    m_tasks = {};
}
