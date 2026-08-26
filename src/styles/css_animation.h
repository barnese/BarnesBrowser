#pragma once

#include <optional>
#include <string>

class CSSAnimation
{
public:
    virtual ~CSSAnimation() = default;
    virtual std::optional<std::string> animate() = 0;
};
