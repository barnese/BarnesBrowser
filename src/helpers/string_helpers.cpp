#include "string_helpers.h"
#include <algorithm>
#include <regex>

// Helper function that splits a texting by a delimiter.
std::vector<std::string> split(std::string text, std::string delimiter, bool firstOnly)
{
    std::vector<std::string> result;

    std::size_t start = 0;
    std::size_t end;

    while ((end = text.find(delimiter, start)) != std::string::npos)
    {
        result.push_back(text.substr(start, end - start));
        start = end + delimiter.length();

        if (firstOnly) 
        {
            break;
        }
    }

    result.push_back(text.substr(start));

    return result;
}

std::string trim(std::string text) 
{
    return trimRight(trimLeft(text));
}

std::string trimLeft(std::string text)
{
    std::size_t start = text.find_first_not_of(" \t\n\r\f\v");

    if (start == std::string::npos) 
        return "";

    return text.substr(start);
}

std::string trimRight(std::string text)
{
    std::size_t end = text.find_last_not_of(" \t\n\r\f\v");
    if (end == std::string::npos)
        return "";
    return text.substr(0, end + 1);
}

std::string toLowercase(std::string text)
{
    std::ranges::transform(text, text.begin(),
        [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        });

    return text;
}

std::string removeComments(std::string text)
{
    // Replace /* */ style comments with an empty string.
    std::regex regex("/\\*(.|[\r\n])*?\\*/");
    return std::regex_replace(text, regex, "");
}
