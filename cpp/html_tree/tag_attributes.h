#pragma once

#include <map>
#include <string>

struct TagAttributes
{
    std::string tag;
    std::map<std::string, std::string> attributes;
};
