#pragma once

#include <chrono>
#include <optional>

struct CachePolicy
{
    bool noStore = false;
    std::optional<std::chrono::seconds> maxAge;

    bool operator==(const CachePolicy& other) const
    {
        return 
            other.noStore == noStore &&
            other.maxAge == maxAge;
    }
};
