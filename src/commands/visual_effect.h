#pragma once

#include "command.h"

/// Abstract class that defines an interface for visual effects commands.
class VisualEffect : public Command
{
public:
    VisualEffect(QRectF r, std::vector<std::unique_ptr<Command>> commands, Node* n)
    {
        node = n;
        rect = r;
        children = std::move(commands);
        for (const auto& child : children)
        {
            // Join the child rect with this rect.
            rect |= child->rect;

            // A visual effect needs compositing if any of its descendants do.
            // If one effect is in the draw phase, then the ones above it will
            // have to be as well.
            if (auto* effect = dynamic_cast<VisualEffect*>(child.get()))
            { 
                if (effect->needsCompositing)
                {                
                    needsCompositing = true;
                }
            }
        }
    }

    QRectF unmap(QRectF rect) const override { return rect; }
};
