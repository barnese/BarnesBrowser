#pragma once

#include <functional>
#include <string>
#include <tuple>

/// Interface for a task
class ITask {
public:
    virtual ~ITask() = default;
    virtual void run() = 0;
    virtual std::string getName() const = 0;
};

/// Defines a generic task for a given function and its arguments.
template<typename... Args>
class Task : public ITask
{
public:
    /// @brief task constructor
    /// @param name - the task name, used for console print statements
    /// @param code - the function to run
    /// @param args - the arguments of that function
    Task(std::string name, std::function<void(Args...)> code, Args... args)
        : m_name(name), 
          m_code(code), 
          m_args(std::make_tuple(args...)
        ) {};

    void run() override
    {
        std::apply(m_code, m_args);
    }

    std::string getName() const override { return m_name; }

private:
    std::string m_name;
    std::function<void(Args...)> m_code;
    std::tuple<Args...> m_args;
};
