#include "iframe_layout.h"

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
    Layout* parent, 
    Layout* previous,
    Frame* parentFrame
) : EmbedLayout(node, parent, previous, parentFrame)
{
}

void IFrameLayout::layout()
{
    if (!layoutNeeded()) return;
    EmbedLayout::layout();
    
    auto zoom = this->zoom.read(&height);

    if (const auto& element = dynamic_cast<Element*>(node))
    {   
        auto widthAttr = element->findAttribute("width");
        auto widthStyle = element->findStyle("width");
        auto heightAttr = element->findAttribute("height");
        auto heightStyle = element->findStyle("height");

        if (widthAttr.has_value())
        {
            width.set(dpx(std::stod(*widthAttr) + 2, zoom));
        }
        else if (widthStyle.has_value())
        {
            width.set(dpx(std::stod(*widthStyle) + 2, zoom));
        }
        else 
        {
            width.set(dpx(Constants::IFRAME_WIDTH_PX + 2, zoom));
        }

        if (heightAttr.has_value())
        {
            height.set(dpx(std::stod(*heightAttr) + 2, zoom));
        }
        else if (heightStyle.has_value())
        {
            height.set(dpx(std::stod(*heightStyle) + 2, zoom));
        }
        else 
        {    
            height.set(dpx(Constants::IFRAME_HEIGHT_PX + 2, zoom));
        }

        if (element->frame && element->frame->isLoaded())
        {
            element->frame->setFrameSize(width.get() - dpx(2, zoom), height.get() - dpx(2, zoom));
            element->frame->document->width.mark();
        }

        ascent.set(height.read(&ascent));
        descent.set(0);
    }
}

std::vector<std::unique_ptr<Command>> IFrameLayout::paint() const
{
    std::vector<std::unique_ptr<Command>> commands;

    if (auto const& element = dynamic_cast<Element*>(node))
    {
        auto bgColor = node->findStyle("background-color", "transparent");
        if (bgColor != "transparent")
        {
            auto borderRadius = element->findStyle("border-radius", "0px");
            auto radius = pixelsToFloat(borderRadius);
            auto color = QString::fromStdString(bgColor);
            
            commands.push_back(
                std::make_unique<DrawRoundedRect>(getRect(), color, radius)
            );
        }
    }

    return commands;
}

std::vector<std::unique_ptr<Command>> IFrameLayout::paintEffects(std::vector<std::unique_ptr<Command>> commands) const
{
    auto zoom = this->zoom.get();
    auto diff = dpx(1, zoom);
    auto rect = getRect();
    QPointF offset(x.get() + diff, y.get() + diff);

    // Simpler way for now. Let paintVisualEffects handle the clipping.
    auto updatedStyles = node->styles.get();
    updatedStyles["overflow"] = "clip";
    node->styles.set(updatedStyles);
    
    // Translate the iframe's contents
    commands = commandList(std::make_unique<Transform>(offset, rect, node, std::move(commands)));

    QRect outlineRect(
        x.get() + diff, 
        y.get() + diff, 
        width.get() - 2 * diff - 2, 
        height.get() - 2 * diff - 2
    );
    paintOutline(node, commands, outlineRect, zoom);

    commands = paintVisualEffects(node, std::move(commands), rect);

    return commands;
}

std::string IFrameLayout::describe() const
{
    std::stringstream s; 
    s << "IFrameLayout(rect: {"
      << x.get() << ", " << y.get() << ", " << width.get() << ", " << height.get()
      << "}, node: " << node->describe() << ")";
    return s.str();
}
