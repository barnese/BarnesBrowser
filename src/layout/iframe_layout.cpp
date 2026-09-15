#include "iframe_layout.h"

#include "commands/blend.h"
#include "commands/draw_rounded_rect.h"
#include "commands/transform.h"
#include "helpers/string_helpers.h"
#include "html_tree/element.h"
#include "layout_helpers.h"
#include "layout_effects_helpers.h"
#include "constants.h"

#include <QPointF>

IFrameLayout::IFrameLayout(
    Node* node, 
    const Layout* parent, 
    const Layout* previous,
    Frame* parentFrame
) : EmbedLayout(node, parent, previous, parentFrame)
{
}

void IFrameLayout::layout()
{
    EmbedLayout::layout();

    if (const auto& element = dynamic_cast<Element*>(node))
    {   
        auto widthAttr = element->findAttribute("width");
        auto heightAttr = element->findAttribute("height");

        if (widthAttr.has_value())
        {
            width = dpx(std::stod(*widthAttr) + 2, zoom);
        }
        else 
        {
            width = dpx(Constants::IFRAME_WIDTH_PX + 2, zoom);
        }

        if (heightAttr.has_value())
        {
            height = dpx(std::stod(*heightAttr) + 2, zoom);
        }
        else 
        {    
            height = dpx(Constants::IFRAME_HEIGHT_PX + 2, zoom);
        }

        if (element->frame && element->frame->isLoaded())
        {
            element->frame->setFrameSize(width - dpx(2, zoom), height - dpx(2, zoom));
        }

        ascent = height;
        descent = 0;
    }
}
std::vector<std::unique_ptr<Command>> IFrameLayout::paint() const
{
    std::vector<std::unique_ptr<Command>> commands;

    if (auto const& element = dynamic_cast<Element*>(node))
    {
        QRect rect(x, y, width, height);
        auto bgColor = node->findStyle("background-color", "transparent");
        if (bgColor != "transparent")
        {
            auto borderRadius = element->findStyle("border-radius", "0px");
            auto radius = pixelsToFloat(borderRadius);
            auto color = QString::fromStdString(bgColor);
            
            commands.push_back(
                std::make_unique<DrawRoundedRect>(rect, color, radius)
            );
        }
    }

    return commands;
}

std::vector<std::unique_ptr<Command>> IFrameLayout::paintEffects(std::vector<std::unique_ptr<Command>> commands) const
{
    auto diff = dpx(1, zoom);
    QRect rect(x, y, width, height);

    // Simpler way for now. Let paintVisualEffects handle the clipping.
    node->styles["overflow"] = "clip";

    QRect outlineRect(x + diff, y + diff, width - 2 * diff, height - 2 * diff);
    paintOutline(node, commands, outlineRect, zoom);

    commands = paintVisualEffects(node, std::move(commands), rect);

    return commands;
}

std::string IFrameLayout::describe() const
{
    std::stringstream s; 
    s << "IFrameLayout(rect: {"
      << x << ", " << y << ", " << width << ", " << height
      << "}, node: " << node->describe() << ")";
    return s.str();
}
