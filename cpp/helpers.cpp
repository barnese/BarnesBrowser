#include "helpers.h"

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