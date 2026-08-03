#pragma once

#include <stdexcept>

class JSContextException : public std::runtime_error
{
public:
    explicit JSContextException(const std::string& message)
        : std::runtime_error(message) {}
};
