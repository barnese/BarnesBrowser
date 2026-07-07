#pragma once

struct Point
{
    int x;
    int y;

    /// Generates all comparison operators (<, <=, >, etc.) at compile time.
    auto operator<=>(const Point&) const = default;
};
