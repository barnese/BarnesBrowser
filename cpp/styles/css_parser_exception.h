#pragma once

#include <stdexcept>

class CSSParserException : public std::runtime_error
{
public:
    explicit CSSParserException(const std::string& message)
        : std::runtime_error(message) {}
};
