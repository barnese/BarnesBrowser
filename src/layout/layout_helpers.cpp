#include "layout_helpers.h"

#include "commands/transform.h"
#include "styles/css_parser.h"

#include <iostream>

double dpx(double cssPx, double zoom)
{
    return cssPx * zoom;
}

QRect absoluteBoundsForObj(Layout* layout)
{
    auto rect = layout->getRect();
    auto current = layout->node;
    while (current)
    {
        auto translation = parseTransform(current->findStyle("transform", ""));
        rect = mapTranslation(rect, translation, false);
        current = current->parent;
    }
    return rect;
}

void printLayoutTree(Layout* layout, int depth)
{
    if (depth == 0)
        std::cout << "Tree Layout:" << std::endl;
    std::string indent(depth * 2, ' ');
    std::cout << indent << layout->describe() << std::endl;
    for (const auto& child : layout->children.getValue())
        printLayoutTree(child.get(), depth + 1);
}

void markLayoutChildrenDirty(Layout* layout)
{
    if (!layout) return;

    layout->children.mark();
    
    Layout* ancestor = layout;
    while (ancestor != nullptr)
    {
        ancestor->hasDirtyDescendants = true;
        ancestor = ancestor->parent;
    }
}
