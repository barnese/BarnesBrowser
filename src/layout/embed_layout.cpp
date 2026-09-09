#include "embed_layout.h"

#include "styles/style_helpers.h"

EmbedLayout::EmbedLayout(
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

void EmbedLayout::layout()
{
    zoom = parent->zoom;
    font = getFont(node, zoom);
    if (previous)
    {
        QFontMetrics previousMetrics(previous->font);
        auto space = previousMetrics.horizontalAdvance(" ");
        x = previous->x + space + previous->width;
    }
    else 
    {
        x = parent->x;
    }
}

bool EmbedLayout::shouldPaint() const
{
    return true;
}

std::vector<std::unique_ptr<Command>> EmbedLayout::paint() const
{
    return {};
}

std::vector<std::unique_ptr<Command>> EmbedLayout::paintEffects(std::vector<std::unique_ptr<Command>>) const
{
    return {};
}

std::string EmbedLayout::describe() const
{
    return "EmbedLayout";
}
