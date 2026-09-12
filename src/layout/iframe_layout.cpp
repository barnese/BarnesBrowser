#include "iframe_layout.h"

#include "commands/blend.h"
#include "commands/draw_rounded_rect.h"
#include "commands/transform.h"
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

std::vector<std::unique_ptr<Command>> IFrameLayout::paintEffects(std::vector<std::unique_ptr<Command>> commands) const
{
    QRect rect(x, y, width, height);
    auto diff = dpx(1, zoom);
    QPointF offset(x + diff, y + diff);
    
    commands.push_back(
        std::make_unique<Transform>(offset, rect, node, std::move(commands))
    );

    QRect innerRect(x + diff, y + diff, width - diff, height - diff);
    
    std::vector<std::unique_ptr<Command>> internalCommands = std::move(commands);

    internalCommands.push_back(
        std::make_unique<Blend>("destination-in", 1, nullptr, 
            commandList(std::make_unique<DrawRoundedRect>(innerRect, "white", 0))
        )
    );

    commands.push_back(
        std::make_unique<Blend>("source-over", 1, node, std::move(internalCommands))
    );

    commands = paintVisualEffects(node, std::move(commands), rect);

    return commands;
}

std::string IFrameLayout::describe() const
{
    return "IFrameLayout";
}