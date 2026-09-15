#include "layout_effects_helpers.h"

#include "commands/blend.h"
#include "commands/draw_outline.h"
#include "commands/draw_rounded_rect.h"
#include "commands/transform.h"
#include "styles/css_parser.h"
#include "helpers/string_helpers.h"
#include "layout_helpers.h"

std::vector<std::unique_ptr<Command>> paintVisualEffects(
    Node* node, 
    std::vector<std::unique_ptr<Command>> commands, 
    QRect rect)
{
    auto opacity = std::stod(node->findStyle("opacity", "1.0"));
    auto blendMode = node->findStyle("mix-blend-mode");
    auto overflow = node->findStyle("overflow");
    auto transformStyle = node->findStyle("transform", "");
    auto translation = parseTransform(transformStyle);

    Blend* overflowMask = nullptr;

    if (overflow == "clip")
    {
        // Apply overflow clipping by creating a new layer, draw the clip 
        // mask into it, then blend it in using "destination-in compositing".
        if (!blendMode.has_value())
        {
            blendMode = "source-over";
        }
        
        auto borderRadius = pixelsToFloat(node->findStyle("border-radius", "0px"));

        auto mask = std::make_unique<Blend>("destination-in", 1.0, nullptr, commandList(
            std::make_unique<DrawRoundedRect>(rect, "white", borderRadius) 
        ));

        overflowMask = mask.get();
        commands.push_back(std::move(mask));
    }

    // Apply the blend and opacity effects to each of the given commands. 
    auto blendOp = std::make_unique<Blend>(
        blendMode, opacity, node, std::move(commands)
    );
    
    node->blendOp = blendOp.get();

    if (overflowMask)
    {
        // Override the overflow mask's rect with the outer blend, which has the
        // union of all content bounds. This is required to get clipping behavior.
        overflowMask->setBlendRect(blendOp->rect);
    }

    auto transform = std::make_unique<Transform>(translation, rect, node, commandList(std::move(blendOp)));
    return commandList(std::move(transform));
}

std::optional<ParseOutlineResult> parseOutline(const std::string& outlineString)
{
    if (outlineString.empty()) return std::nullopt;

    auto values = split(outlineString, " ", false);

    if (values.size() != 3) return std::nullopt;
    if (values[1] != "solid") return std::nullopt;

    auto thickness = pixelsToInt(values[0]);
    if (!thickness.has_value()) return std::nullopt;

    auto color = values[2];

    return ParseOutlineResult(thickness.value(), color);
}

void paintOutline(Node* node, std::vector<std::unique_ptr<Command>>& commands, QRect rect, double zoom)
{
    if (!node) return;

    auto outline = parseOutline(node->findStyle("outline", ""));
    
    if (!outline.has_value()) return;
    
    commands.push_back(
        std::make_unique<DrawOutline>(
            rect, (*outline).color, 
            dpx((*outline).thickness, zoom)
        )
    );
}
