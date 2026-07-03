#include "string_helpers.h"

// Helper function that splits a string by a delimiter.
std::vector<std::string> split(const std::string& str, const std::string& delimiter)
{
    std::vector<std::string> result;

    std::size_t start = 0;
    std::size_t end;

    while ((end = str.find(delimiter, start)) != std::string::npos)
    {
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }

    result.push_back(str.substr(start));

    return result;
}

std::string trim(const std::string& str) 
{
    return trimRight(trimLeft(str));
}

std::string trimLeft(const std::string& str)
{
    std::size_t start = str.find_first_not_of(" \t\n\r\f\v");

    if (start == std::string::npos) 
        return "";

    return str.substr(start);
}

std::string trimRight(const std::string& str)
{
    std::size_t end = str.find_last_not_of(" \t\n\r\f\v");
    if (end == std::string::npos)
        return "";
    return str.substr(0, end + 1);
}