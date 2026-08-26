#include "layout_effects_helpers.h"

#include "commands/blend.h"
#include "commands/draw_rounded_rect.h"
#include "helpers/string_helpers.h"

std::vector<std::unique_ptr<Command>> paintVisualEffects(
    Node* node, 
    std::vector<std::unique_ptr<Command>> commands, 
    QRect rect)
{
    auto opacity = std::stod(node->findStyle("opacity", "1.0"));
    auto blendMode = node->findStyle("mix-blend-mode");
    auto overflow = node->findStyle("overflow");

    // Apply overflow clipping by creating a new layer, draw the clip 
    // mask into it, then blend it in using "destination-in compositing".
    if (overflow == "clip")
    {
        if (!blendMode.has_value())
        {
            blendMode = "source-over";
        }
        
        auto borderRadius = pixelsToFloat(node->findStyle("border-radius", "0px"));

        commands.push_back(
            std::make_unique<Blend>("destination-in", opacity, nullptr, commandList(
                std::make_unique<DrawRoundedRect>(rect, "white", borderRadius) 
            ))
        );
    }

    // Apply the blend and opacity effects to each of the given commands. 
    auto blendOp = std::make_unique<Blend>(blendMode, opacity, node, std::move(commands));
    node->blendOp = blendOp.get();
    return commandList(std::move(blendOp));
}
