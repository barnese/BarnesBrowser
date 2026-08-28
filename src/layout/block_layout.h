#pragma once

#include "layout.h"
#include "html_tree/node.h"

#include <map>
#include <memory>
#include <tuple>
#include <QString>
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
    BlockLayout(Node* node, const Layout* parent, const Layout* previous = nullptr);
    void layout() override;
    std::vector<std::unique_ptr<Command>> paint() const override;
    bool shouldPaint() const override;
    std::vector<std::unique_ptr<Command>> paintEffects(std::vector<std::unique_ptr<Command>> commands) const override;
    std::string describe() const override;
    
private:
    BlockLayoutMode getLayoutMode() const;
    void layoutBlock();
    void layoutText();
    bool areAnyChildBlockElements() const;
    void recurse(Node* node);
    void processInput(Node* node);
    void processWord(Node* node, QString& word);
    void newLine();
    QFont getFont(QString family, int size, int weight, int style);

    int m_cursorX;

    // Font cache indexed by family, size, weight, style.
    std::map<std::tuple<QString, int, int, int>, QFont> m_fonts;
};
