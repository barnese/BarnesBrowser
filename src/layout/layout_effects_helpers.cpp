#include "layout_effects_helpers.h"

#include "commands/blend.h"
#include "commands/opacity.h"

std::vector<std::unique_ptr<Command>> paintVisualEffects(
    Node* node, 
    std::vector<std::unique_ptr<Command>> commands, 
    QRect rect
)
{
    auto opacity = std::stod(node->findStyle("opacity", "1.0"));
    auto blendMode = node->findStyle("mix-blend-mode", "source-over");

    auto opacityCommand = std::make_unique<Opacity>(opacity, std::move(commands));
    std::vector<std::unique_ptr<Command>> blendCmds;
    blendCmds.push_back(std::move(opacityCommand));
    auto blendCommand = std::make_unique<Blend>(blendMode, std::move(blendCmds));

    std::vector<std::unique_ptr<Command>> effects;
    effects.push_back(std::move(blendCommand));
    return effects;
}
