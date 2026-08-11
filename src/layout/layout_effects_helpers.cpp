#include "layout_effects_helpers.h"

#include "commands/blend.h"
#include "commands/draw_rounded_rect.h"
#include "commands/opacity.h"
#include "helpers/string_helpers.h"

std::vector<std::unique_ptr<Command>> paintVisualEffects(
    Node* node, 
    std::vector<std::unique_ptr<Command>> commands, 
    QRect rect)
{
    auto opacity = std::stod(node->findStyle("opacity", "1.0"));
    auto blendMode = node->findStyle("mix-blend-mode", "source-over");
    auto overflow = node->findStyle("overflow", "");
    auto borderRadius = pixelsToFloat(node->findStyle("border-radius", "0px"));

    // Apply overflow clipping by creating a new layer, draw the clip 
    // mask into it, then blend it in using "destination-in compositing".
    if (overflow == "clip")
    {
        commands.push_back(
            std::make_unique<Blend>("destination-in", commandList(
                std::make_unique<DrawRoundedRect>(rect, "white", borderRadius) 
            ))
        );
    }

    // First, apply the opacity effect by wrapping the provided 
    // layout commands with this new opacity command. Then,
    // apply the blend effect to support the mix-bend-mode style,
    // using the commandList helper to improve readability.
    return commandList(
        std::make_unique<Blend>(blendMode, commandList(
            std::make_unique<Opacity>(opacity, std::move(commands))
        ))
    );
}
