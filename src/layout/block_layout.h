#pragma once

#include "layout.h"
#include "html_tree/node.h"
#include "html_tree/text.h"
#include "point.h"

#include <map>
#include <memory>
#include <tuple>
#include <QGraphicsScene>

enum BlockLayoutMode {
    INLINE,
    BLOCK
};

/// Handles block layouts of an HTML page, where a "block" is a
/// paragraph, header or other group of text. Specifically, this
/// class updates the layout's display list for the given root node.
/// Layout operates using page coordinates, which represent 
/// an entire page, not only what is visible on the screen.
class BlockLayout : public Layout
{
public:
    BlockLayout(const Node* node, const Layout* parent, const Layout* previous = nullptr);
    void layout() override;
    std::vector<std::unique_ptr<Command>> paint() const override;
    
private:
    BlockLayoutMode getLayoutMode() const;
    void layoutBlock();
    void layoutText();
    bool areAnyChildBlockElements() const;
    void recurse(const Node* node);
    void processWord(const Node* node, const QString& word);
    void flush();
    std::tuple<int, int> getMaxAscentAndDescentForLine() const;
    QFont getFont(int size, int weight, int style);

    int m_cursorX;
    int m_cursorY;
    QString m_family;

    // List of display content with layout processing applied.
    std::map<Point, std::tuple<QString, QFont, QString>> m_displayList;

    // A temporary buffer for a line of display content: x, word, font, color.
    std::vector<std::tuple<int, QString, QFont, QString>> m_line;
    
    // Font cache indexed by size, weight, style.
    std::map<std::tuple<int, int, int>, QFont> m_fonts;
};
