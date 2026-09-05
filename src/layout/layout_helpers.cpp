#include "layout_helpers.h"

#include "commands/transform.h"
#include "styles/css_parser.h"

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
