#pragma once

#include "layout.h"
#include "html_tree/node.h"

#include <map>
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
    DisplayList paint() const override;
    
private:
    BlockLayoutMode getLayoutMode();
    void layoutBlock();
    void layoutText();
    bool areAnyChildBlockElements();
    void recurse(const Node* node);
    void processWord(const QString& word);
    void processOpenTag(const std::string& tag);
    void processCloseTag(const std::string& tag);
    void flush();
    std::tuple<int, int> getMaxAscentAndDescentForLine() const;
    QFont getFont(int size, int weight, int style);

    const std::vector<std::string> BLOCK_ELEMENTS = 
    {
        "html", "body", "article", "section", "nav", "aside",
        "h1", "h2", "h3", "h4", "h5", "h6", "hgroup", "header",
        "footer", "address", "p", "hr", "pre", "blockquote",
        "ol", "ul", "menu", "li", "dl", "dt", "dd", "figure",
        "figcaption", "main", "div", "table", "form", "fieldset",
        "legend", "details", "summary"
    };

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
