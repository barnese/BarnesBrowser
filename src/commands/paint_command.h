#pragma once

#include "command.h"

/// Command that helps to separate painting commands from other 
/// commmands, such as VisualEffects.
class PaintCommand : public Command
{
    QRectF map(QRectF rect) const override 
    { 
        return rect; 
    }
    
    QRectF unmap(QRectF rect) const override
    { 
        return rect; 
    }
    
    std::unique_ptr<Command> clone(std::unique_ptr<Command>) override 
    { 
        return nullptr; 
    };
};
