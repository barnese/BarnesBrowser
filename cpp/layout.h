#pragma once

#include <map>
#include <tuple>
#include "point.h"
#include "html_tree/node.h"
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
    Layout(const Node* node);
    DisplayList getDisplayList() const;

private:
    void recurse(const Node* node);
    void processWord(const QString& word);
    void processOpenTag(const std::string& tag);
    void processCloseTag(const std::string& tag);
    void flush();
    std::tuple<int, int> getMaxAscentAndDescentForLine() const;
    QFont getFont(int size, int weight, int style);

    int m_cursorX;
    int m_cursorY;
    QString m_family;
    int m_size;
    int m_weight;
    int m_style;

    // List of display content with layout processing applied.
    DisplayList m_displayList;
    
    // A temporary buffer for a line of display content.
    std::vector<std::tuple<int, QString, QFont>> m_line;
    
    // Font cache indexed by size, weight, style.
    std::map<std::tuple<int, int, int>, QFont> m_fonts;
};
