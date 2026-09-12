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
    QRect rect(layout->x, layout->y, layout->width, layout->height);
    auto current = layout->node;
    while (current)
    {
        auto translation = parseTransform(current->findStyle("transform", ""));
        rect = mapTranslation(rect, translation, false);
        current = current->parent;
    }
    return rect;
}

void printLayoutTree(const Layout* layout, int depth)
{
    if (depth == 0)
        std::cout << "Tree Layout:" << std::endl;
    std::string indent(depth * 2, ' ');
    std::cout << indent << layout->describe() << std::endl;
    for (const auto& child : layout->children)
        printLayoutTree(child.get(), depth + 1);
}
