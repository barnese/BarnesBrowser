#pragma once

#include <compare>
#include <sstream>
#include <string>

struct Point
{
    int x;
    int y;

    /// Generates all comparison operators (<, <=, >, etc.) at compile time.
    auto operator<=>(const Point&) const = default;

    std::string describe()
    {
        std::stringstream s; 
        s << "Point(" << x << ", " << y << ")";
        return s.str();
    }
};
