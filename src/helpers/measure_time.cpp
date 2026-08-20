#include "measure_time.h"

#include "helpers/thread_registry.h"

#include <chrono>
#include <thread>

/// Returns the current epoch time in microseconds (1/1,000,000 second).
long epochTime()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

MeasureTime::MeasureTime()
{
    std::lock_guard lock(m_mutex);
    
    m_file = std::ofstream("browser.trace");
    
    m_file << R"({"traceEvents": [)"
           << R"({"name": "process_name", )"
           << R"("ph": "M", )"
           << R"("ts": )" << epochTime() << ", "
           << R"("pid": 1, )"
           << R"("cat": "__metadata", )"
           << R"("args": {"name": "BBrowser"})"
           << "}";
}

void MeasureTime::start(const std::string& name)
{
    std::lock_guard lock(m_mutex);

    auto tid = std::this_thread::get_id();

    m_file << R"(, {"ph": "B", "cat": "_", )"
           << R"("name": ")" << name << R"(", )"
           << R"("ts": )" << epochTime() << ", "
           << R"("pid": 1, "tid": ")" << tid << R"("})";
}

void MeasureTime::stop(const std::string& name)
{
    std::lock_guard lock(m_mutex);
 
    auto tid = std::this_thread::get_id();
    
    m_file << R"(, {"ph": "E", "cat": "_", )"
           << R"("name": ")" << name << R"(", )"
           << R"("ts": )" << epochTime() << ", "
           << R"("pid": 1, "tid": ")" << tid << R"("})";
}

void MeasureTime::finish()
{
    std::lock_guard lock(m_mutex);
    
    // Before finishing the file, enumerate the current threads and
    // add metadata events containing human-readable thread names.
    for (const auto& thread : threadRegistry().list())
    {
        auto tid = thread.id;
        auto name = thread.name;

        m_file << R"(, {"ph": "M", "name": "thread_name", )"
               << R"("pid": 1, "tid": ")" << tid << R"(", )"
               << R"("args": {"name": ")" << name
               << R"("}})";
    }

    m_file << "]}";
    m_file.close();
}
