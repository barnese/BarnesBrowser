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
    
    zoom.initialize(this, "zoom", parent, DepList{&parent->zoom});
    font.initialize(this, "font", parent, DepList{
        &zoom,
        &node->styles["font-family"],
        &node->styles["font-weight"],
        &node->styles["font-style"],
        &node->styles["font-size"]
    });
    width.initialize(this, "width", parent, DepList{&zoom});
    height.initialize(this, "height", parent, DepList{&zoom, &font, &width});
    ascent.initialize(this, "ascent", parent, DepList{&height});
    descent.initialize(this, "descent", parent, DepList{});
    
    DepList xDependencies;
    if (previous)
    {
        xDependencies = {&previous->x, &previous->font, &previous->width};
    }
    else 
    {
        xDependencies = {&parent->x};
    }
    x.initialize(this, "x", parent, xDependencies);
    y.initialize(this, "y", parent, DepList{&ascent, &parent->y, &parent->ascent});
 
    hasDirtyDescendants = true;
}

bool EmbedLayout::layoutNeeded() const
{
    if (zoom.isDirty()) return true;
    if (width.isDirty()) return true;
    if (height.isDirty()) return true;
    if (x.isDirty()) return true;
    if (y.isDirty()) return true;
    if (ascent.isDirty()) return true;
    if (descent.isDirty()) return true;
    if (font.isDirty()) return true;
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
