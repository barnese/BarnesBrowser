#pragma once

#include "command.h"

/// Command that helps to separate painting commands from other 
/// commmands, such as VisualEffects.
class PaintCommand : public Command
{
};
