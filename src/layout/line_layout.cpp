#include "line_layout.h"

#include <QFontMetrics>

LineLayout::LineLayout(
    const Node* node, 
    const Layout* parent, 
    const Layout* previous
)
{
    this->node = node;
    this->parent = parent;
    this->previous = previous;
}

void LineLayout::layout()
{
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
