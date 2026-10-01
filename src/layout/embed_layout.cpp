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
    
    zoom = ProtectedField<double>(this, "zoom");
    font = ProtectedField<QFont>(this, "font");
    width = ProtectedField<double>(this, "width");
    height = ProtectedField<double>(this, "height");
    x = ProtectedField<double>(this, "x");
    y = ProtectedField<double>(this, "y");
    ascent = ProtectedField<int>(this, "ascent");
    descent = ProtectedField<int>(this, "descent");
 
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

    font.set(getFont(node, zoom.read(&font), 0, &font));

    if (previous)
    {
        auto prevFont = previous->font.read(&x);
        QFontMetrics previousMetrics(prevFont);
        auto space = previousMetrics.horizontalAdvance(" ");
        auto prevX = previous->x.read(&x);
        auto prevWidth = previous->width.read(&x);
        x.set(prevX + space + prevWidth);
    }
    else 
    {
        x.copy(&parent->x);
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
