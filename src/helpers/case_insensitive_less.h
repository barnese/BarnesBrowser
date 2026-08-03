#pragma once

#include <algorithm>
#include <string>

/// Defines a case insensitive comparator, useful when you want key
/// lookups in an std::map to be case insensitive.
struct CaseInsensitiveLess
{
    bool operator()(const std::string& a, const std::string& b) const
    {
        return std::lexicographical_compare(
            a.begin(), a.end(),
            b.begin(), b.end(),
            [](unsigned char c1, unsigned char c2)
            {
                return std::tolower(c1) < std::tolower(c2);
            });
    }
};
