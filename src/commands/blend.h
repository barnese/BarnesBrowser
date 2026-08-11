#pragma once

#include "command.h"

#include <QString>
#include <memory>
#include <string>
#include <vector>

/// Defines a command for implementing the blend mode visual effect.
class Blend : public Command
{
public:
    Blend(std::string blendMode, std::vector<std::unique_ptr<Command>> children);
    void execute(QImage* surface) override;
    std::string describe() const override;

private:
    
    std::string m_blendMode;
    std::vector<std::unique_ptr<Command>> m_children;
};
