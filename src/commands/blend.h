#pragma once

#include "visual_effect.h"

#include <QString>
#include <memory>
#include <optional>
#include <string>
#include <vector>

/// Defines a command for implementing mix-blend-mode and opacity visual effects.
class Blend : public VisualEffect
{
public:
    Blend(
        std::optional<std::string> blendMode, 
        double opacity, 
        std::vector<std::unique_ptr<Command>> children
    );
    void execute(PaintContext& context) override;
    std::string describe() const override;
    std::unique_ptr<Command> clone(std::unique_ptr<Command> child) override;

private:
    std::optional<std::string> m_blendMode;
    double m_opacity;
};
