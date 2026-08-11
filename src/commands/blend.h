#pragma once

#include "command.h"

#include <QString>
#include <memory>
#include <optional>
#include <string>
#include <vector>

/// Defines a command for implementing mix-blend-mode and opacity visual effects.
class Blend : public Command
{
public:
    Blend(
        std::optional<std::string> blendMode, 
        double opacity, 
        std::vector<std::unique_ptr<Command>> children
    );
    void execute(QImage* surface) override;
    std::string describe() const override;

private:
    
    std::optional<std::string> m_blendMode;
    double m_opacity;
    std::vector<std::unique_ptr<Command>> m_children;
};
