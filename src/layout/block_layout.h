#pragma once

#include "layout.h"
#include "html_tree/node.h"
#include "styles/style_helpers.h"

#include <memory>
#include <QRect>
#include <QString>

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
    BlockLayout(Node* node, Layout* parent, Layout* previous, Frame* frame);
    bool layoutNeeded() const override;
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
    void iframe(Node* node);
    void image(Node* node);
    void input(Node* node);
    void word(Node* node, QString& word);
    void newLine();
    
    template<typename T>
    void addInlineChild(Node* node, int wordWidth, Frame* frame, std::optional<QString> word = std::nullopt)
    {
        auto width = this->width.read(&children);

        if (m_cursorX + wordWidth > width)
            newLine();

        auto line = tempChildren.back().get();
        std::unique_ptr<Layout> child = nullptr;

        if (word.has_value())
        { 
            if constexpr (std::constructible_from<T, Node*, QString, Layout*, Layout*>) {
                child = std::make_unique<T>(node, *word, line, previousWord);
            }
        }
        else 
        {
            if constexpr (std::constructible_from<T, Node*, Layout*, Layout*, Frame*>) {
                child = std::make_unique<T>(node, line, previousWord, frame);
            }        
        }

        if (child)
        {
            previousWord = child.get();
            line->children.getValue().push_back(std::move(child));
        }

        auto zoom = this->zoom.read(&children);

        const QFont font = getFont(node, zoom, 0, &children);
        const QFontMetrics metrics(font);
        const auto spaceWidth = metrics.horizontalAdvance(" ");
        m_cursorX += wordWidth + spaceWidth;
    }

    int m_cursorX;
    std::vector<std::unique_ptr<Layout>> tempChildren;
    Layout* previousWord;
};
