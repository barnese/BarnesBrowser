#pragma once

#include "command.h"

/// Command that helps to separate painting commands from other 
/// commmands, such as VisualEffects.
class PaintCommand : public Command
{
    QRect map(QRect rect) const override 
    { 
        return rect; 
    }
    
    QRect unmap(QRect rect) const override
    { 
        return rect; 
    }
    
    std::unique_ptr<Command> clone(std::unique_ptr<Command>) override 
    { 
        return nullptr; 
    };
};
