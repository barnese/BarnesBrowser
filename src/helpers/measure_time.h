#pragma once

#include <fstream>
#include <mutex>
#include <string>

class MeasureTime
{
public:
    MeasureTime();
    void start(const std::string& name);
    void stop(const std::string& name);
    void finish();

private:
    std::ofstream m_file;
    std::mutex m_mutex;
};
