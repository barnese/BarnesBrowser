#pragma once

#include <iostream>
#include <mutex>
#include <string>
#include <sstream>
#include <thread>

class Logger
{
public:
    Logger(std::string tag) 
        : m_tag(tag) {};

    void info(const std::string& message)
    {
        std::lock_guard lock(m_mutex);
        std::stringstream s;
        s << "\033[0m"; //default color
        s << "[" << m_tag << "] (";
        s << std::this_thread::get_id();
        s << ") " << message << std::endl;
        std::cout << s.str();
    }

    void error(const std::string& message)
    {
        std::lock_guard lock(m_mutex);
        std::stringstream s;
        s << "\033[31m"; // red color
        s << "[" << m_tag << "] (";
        s << std::this_thread::get_id();
        s << ") " << message << std::endl;
        std::cerr << s.str();
    }

private:
    std::string m_tag;
    std::mutex m_mutex;
};
