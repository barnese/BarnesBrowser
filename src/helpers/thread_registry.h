#pragma once

#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct ThreadInfo
{
    std::thread::id id;
    std::string name;
};

class ThreadRegistry
{
public:
    void add(std::thread::id id, std::string name)
    {
        std::lock_guard lock(m_mutex);
        m_threads.push_back({id, std::move(name)});
    }

    void remove(std::thread::id id)
    {
        std::lock_guard lock(m_mutex);
        std::erase_if(m_threads, [id](const ThreadInfo& t) {
            return t.id == id;
        });
    }

    std::vector<ThreadInfo> list() const
    {
        std::lock_guard lock(m_mutex);
        return m_threads;
    }

private:
    mutable std::mutex m_mutex;
    std::vector<ThreadInfo> m_threads;
};

/// Single shared registry of app thread
// info, lazily initialized on first use.
ThreadRegistry& threadRegistry();
