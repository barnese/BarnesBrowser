#pragma once

#include "command.h"

/// Abstract class that defines an interface for visual effects commands.
class VisualEffect : public Command
{
public:
    VisualEffect(QRect r, std::vector<std::unique_ptr<Command>> commands) 
    {
        rect = r;
        children = std::move(commands);
        for (const auto& child : children)
        {
            // Join the child rect with this rect.
            rect |= child->rect;
        }
    }
};
