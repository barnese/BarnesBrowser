#pragma once

#include <map>
#include "point.h"
#include "element.h"
#include <QGraphicsScene>

// Aliases for convenience
using DisplayList = std::map<Point, std::tuple<QString, QFont>>;

/// Handles layout of an HTML page. Specifically, this class
/// updates the layout's display list for the given list of elements.
/// Layout operates using page coordinates, which represent 
/// an entire page, not only what is visible on the screen.
class Layout
{
public:
    Layout(std::vector<Element> elements);
    DisplayList getDisplayList() const;

private:
    void processElement(const Element& element);
    void processWord(const QString& word);

    int m_cursorX;
    int m_cursorY;
    int m_weight;
    int m_style;
    DisplayList m_displayList;
};
