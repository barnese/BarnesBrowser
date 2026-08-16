#include "measure_time.h"

#include <chrono>

/// Returns the current epoch time in microseconds (1/1,000,000 second).
long epochTime()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

MeasureTime::MeasureTime()
{
    m_file = std::ofstream("browser.trace");
    m_file << R"({"traceEvents": [)";
    m_file << R"({"name": "process_name", )";
    m_file << R"("ph": "M", )";
    m_file << R"("ts": )" << epochTime() << ", ";
    m_file << R"("pid": 1, )";
    m_file << R"("cat": "__metadata", )";
    m_file << R"("args": {"name": "BBrowser"})";
    m_file << "}";
}

void MeasureTime::start(const std::string& name)
{
    m_file << R"(, {"ph": "B", "cat": "_", )";
    m_file << R"("name": ")" << name << R"(", )";
    m_file << R"("ts": )" << epochTime() << ", ";
    m_file << R"("pid": 1, "tid": 1})";
}

void MeasureTime::stop(const std::string& name)
{
    m_file << R"(, {"ph": "E", "cat": "_", )";
    m_file << R"("name": ")" << name << R"(", )";
    m_file << R"("ts": )" << epochTime() << ", ";
    m_file << R"("pid": 1, "tid": 1})";
}

void MeasureTime::finish()
{
    m_file << "]}";
    m_file.close();
}
