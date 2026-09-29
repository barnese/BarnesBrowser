#include "embed_layout.h"

#include "styles/style_helpers.h"

EmbedLayout::EmbedLayout(
    Node* node, 
    Layout* parent, 
    Layout* previous,
    Frame* frame
)
{
    this->node = node;
    this->parent = parent;
    this->previous = previous;
    this->frame = frame;
    this->node->layoutObject = this;
    hasDirtyDescendants = true;
}

bool EmbedLayout::layoutNeeded() const
{
    if (zoom.isDirty()) return true;
    if (width.isDirty()) return true;
    if (hasDirtyDescendants) return true;
    return false;
}

void EmbedLayout::layout()
{
    zoom.copy(&parent->zoom);
    // TODO: change this to a read(&font);
    auto zoomValue = zoom.get();

    font = getFont(node, zoomValue);
    if (previous)
    {
        QFontMetrics previousMetrics(previous->font);
        auto space = previousMetrics.horizontalAdvance(" ");
        x = previous->x + space + previous->width.get(); // TODO: should be a read(&x)
    }
    else 
    {
        x = parent->x;
    }

    hasDirtyDescendants = false;
}

bool EmbedLayout::shouldPaint() const
{
    return true;
}

std::vector<std::unique_ptr<Command>> EmbedLayout::paint() const
{
    return {};
}

std::vector<std::unique_ptr<Command>> EmbedLayout::paintEffects(std::vector<std::unique_ptr<Command>> commands) const
{
    return commands;
}

std::string EmbedLayout::describe() const
{
    return "EmbedLayout";
}
