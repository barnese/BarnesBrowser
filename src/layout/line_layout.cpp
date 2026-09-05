#include "line_layout.h"

#include "layout/layout_effects_helpers.h"

#include <QFontMetrics>
#include <sstream>

LineLayout::LineLayout(
    Node* node, 
    const Layout* parent, 
    const Layout* previous
)
{
    this->node = node;
    this->parent = parent;
    this->previous = previous;
    this->node->layoutObject = this;
}

void LineLayout::layout()
{
    zoom = parent->zoom;
    width = parent->width;
    x = parent->x;

    if (previous)
    {
        y = previous->y + previous->height;
    }
    else 
    {
        y = parent->y;
    }

    int maxAscent = 0;
    int maxDescent = 0;

    for (const auto& word : children)
    {
        word->layout();
        
        QFontMetrics metrics(word->font);   
        if (metrics.ascent() > maxAscent)
        {
            maxAscent = metrics.ascent();
        }
        if (metrics.descent() > maxDescent)
        {
            maxDescent = metrics.descent();
        }
    }

    int baseline = y + 1.25 * maxAscent;

    for (auto& word : children)
    {
        QFontMetrics metrics(word->font);   
        word->y = baseline - metrics.ascent();
    }

    height = 1.25 * (maxAscent + maxDescent);
}

bool LineLayout::shouldPaint() const 
{
    return true;
}

std::vector<std::unique_ptr<Command>> LineLayout::paint() const
{
    // There is nothing to paint for a line.
    return {};
}

std::vector<std::unique_ptr<Command>> LineLayout::paintEffects(std::vector<std::unique_ptr<Command>> commands) const
{
    // Add an outline draw command if focused.
    QRect outlineRect;
    Node* outlineNode = nullptr;

    for (const auto& child : children)
    {
        auto outlineString = child->node->parent->findStyle("outline", "");

        if (outlineString.empty()) continue;

        if (parseOutline(outlineString).has_value())
        {
            outlineRect |= child->getRect();
            outlineNode = child->node->parent;
        }
    }

    if (outlineNode != nullptr)
    {
        paintOutline(outlineNode, commands, outlineRect, zoom);
    }

    return commands;
}

std::string LineLayout::describe() const
{
    std::stringstream s; 
    s << "LineLayout(rect: {"
      << x << ", " << y << ", " << width << ", " << height
      << "}, node: " << node->describe() << ")";
    return s.str();
}
