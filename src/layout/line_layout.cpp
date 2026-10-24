#include "line_layout.h"

#include "layout/layout_effects_helpers.h"
#include "layout/text_layout.h"

#include <algorithm>
#include <QFontMetrics>
#include <sstream>

LineLayout::LineLayout(
    Node* node, 
    Layout* parent, 
    Layout* previous
)
{
    this->node = node;
    this->parent = parent;
    this->previous = previous;
    this->node->layoutObject = this;
    
    zoom.initialize(this, "zoom", parent, DepList{&parent->zoom});
    x.initialize(this, "x", parent, DepList{&parent->x});
    std::vector<Invalidatable*> yDependencies;
    if (previous)
    {
        yDependencies = {&previous->y, &previous->height};
    }
    else {
        yDependencies = {&parent->y};
    }
    y.initialize(this, "y", parent, yDependencies);
    initializedFields = false;
    ascent.initialize(this, "ascent", parent);
    descent.initialize(this, "descent", parent);
    width.initialize(this, "width", parent, DepList{&parent->width});
    height.initialize(this, "height", parent, DepList{&ascent, &descent});
    
    hasDirtyDescendants = true;
}

bool LineLayout::layoutNeeded() const
{
    if (zoom.isDirty()) return true;
    if (width.isDirty()) return true;
    if (height.isDirty()) return true;
    if (x.isDirty()) return true;
    if (y.isDirty()) return true;
    if (ascent.isDirty()) return true;
    if (descent.isDirty()) return true;
    if (hasDirtyDescendants) return true;
    return false;
}

void LineLayout::layout()
{
    if (!initializedFields)
    {
        DepList ascentDeps;
        DepList descentDeps;

        for (auto& child : children.getValue())
        {
            ascentDeps.push_back(&child->ascent);
            descentDeps.push_back(&child->descent);
        }
      
        ascent.setDependenciesAndFreeze(ascentDeps);
        descent.setDependenciesAndFreeze(descentDeps);
        
        initializedFields = true;
    }

    if (!layoutNeeded()) return;

    zoom.copy(&parent->zoom);
    width.copy(&parent->width);
    x.copy(&parent->x);

    if (previous)
    {
        auto prevY = previous->y.read(&y);
        auto prevHeight = previous->height.read(&y);
        y.set(prevY + prevHeight);
    }
    else 
    {
        y.copy(&parent->y);
    }

    for (const auto& child : children.getValue())
    {
        child->layout();
    }

    if (children.isEmpty())
    {
        ascent.set(0);
        descent.set(0);
        height.set(0);
        hasDirtyDescendants = false;
        return;
    }

    // Note: in Qt, the ascent is the positive distance from the baseline
    // upward, and the descent is the positive distance downward. Hence:
    //     y-above-baseline = baseline - ascent
    //     y-below-baseline = baseline + descent
    // This differs from Skia, so the signs here are different.

    int maxAscent = children.getValue().front()->ascent.read(&ascent);
    for (const auto& child : children.getValue())
    {
        maxAscent = std::max(maxAscent, child->ascent.read(&ascent));
    }
    ascent.set(maxAscent);

    int maxDescent = children.getValue().front()->descent.read(&descent);
    for (const auto& child : children.getValue())
    {
        maxDescent = std::max(maxDescent, child->descent.read(&descent));
    }
    descent.set(maxDescent);

    for (const auto& child : children.getValue())
    {
        int baseline = y.read(&child->y) + maxAscent;
        if (const auto& text = dynamic_cast<TextLayout*>(child.get()))
        {
            child->y.set(baseline - child->ascent.read(&child->y) / 1.25);
        }
        else 
        {
            child->y.set(baseline - child->ascent.read(&child->y));
        }
    }

    height.set(maxAscent + maxDescent);

    hasDirtyDescendants = false;
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
    QRectF outlineRect;
    Node* outlineNode = nullptr;

    for (const auto& child : children.getValue())
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
        paintOutline(outlineNode, commands, outlineRect, zoom.get());
    }

    return commands;
}

std::string LineLayout::describe() const
{
    std::stringstream s; 
    s << "LineLayout(rect: {"
      << x.get() << ", " << y.get() << ", " << width.get() << ", " << height.get()
      << "}, node: " << node->describe() << ")";
    return s.str();
}
