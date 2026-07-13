#include "block_layout.h"

#include "commands/draw_rect.h"
#include "commands/draw_text.h"
#include "constants.h"
#include "html_tree/element.h"
#include "html_tree/text.h"
#include "layout.h"

#include <QFontMetrics>
#include <memory>

BlockLayout::BlockLayout(const Node* node, const Layout* parent, const Layout* previous)
{
    this->node = node;
    this->parent = parent;
    this->previous = previous;
    x = 0;
    y = 0;
    width = 0;
    height = 0;
}

/// Contructs the layout tree for the current tree node.
void BlockLayout::layout()
{
    x = parent->x;
    width = parent->width;

    if (previous)
        y = previous->y + previous->height;
    else 
        y = parent->y;

    auto mode = getLayoutMode();
    if (mode == BLOCK)
        layoutBlock();
    else 
        layoutText();

    for (const auto& child : children)
        child->layout();

    if (mode == BLOCK)
    {
        height = 0;
        for (const auto& child : children)
            height += child->height;
    }
    else 
    {
        height = m_cursorY;
    }
}

void BlockLayout::layoutBlock()
{
    previous = nullptr;
    for (const auto& child : node->getChildren())
    {
        auto next = std::make_unique<BlockLayout>(child.get(), this, previous);
        previous = next.get();
        children.push_back(std::move(next));
    }
}

void BlockLayout::layoutText() 
{
    m_cursorX = 0;
    m_cursorY = 0;
    m_family = "Helvetica";
    m_style = QFont::Style::StyleNormal;
    m_weight = QFont::Weight::Normal;
    m_size = 16;

    m_line = {};
    recurse(node);
    flush(); 
}

/// Determines if any children contain block elements.
bool BlockLayout::areAnyChildBlockElements() const
{
    for (const auto& child : node->getChildren())
    {
        if (auto element = dynamic_cast<const Element*>(child.get()))
        {
            if (std::ranges::contains(BLOCK_ELEMENTS, element->tag))
            {
                return true;
            }
        }
    }

    return false;
}

/// Determines what layout mode we're using based on our HTML node.
BlockLayoutMode BlockLayout::getLayoutMode() const
{
    if (dynamic_cast<const Text*>(node))
    {
        return INLINE;
    }
    else if (areAnyChildBlockElements()) 
    {
        return BLOCK;
    }
    else if (!node->getChildren().empty())
    {
        return INLINE;
    }
    else 
    {
        return BLOCK;
    }
}

void BlockLayout::recurse(const Node* node) 
{
    if (auto element = dynamic_cast<const Element*>(node)) 
    {
        processOpenTag(element->tag);
        for (const auto& child : element->getChildren())
        {
            recurse(child.get());
        }
        processCloseTag(element->tag);
    }
    else if (auto text = dynamic_cast<const Text*>(node))
    {
        QString content(text->getText().c_str());
        auto words = content.split(' ');

        for (const auto& word : words)
        {
            processWord(word);   
        }
    }
}

void BlockLayout::processOpenTag(const std::string& tag)
{
    if (tag == "i")
        m_style = QFont::Style::StyleItalic;
    else if (tag == "b")
        m_weight = QFont::Weight::Bold;
    else if (tag == "small")
        m_size -= 2;
    else if (tag == "big")
        m_size += 4;
    else if (tag == "br")
        flush();
}
void BlockLayout::processCloseTag(const std::string& tag)
{
    if (tag == "i")
        m_style = QFont::Style::StyleNormal;
    else if (tag == "b")
        m_weight = QFont::Weight::Normal;
    else if (tag == "small")
        m_size += 2;
    else if (tag == "big")
        m_size -= 4;
    else if (tag == "p")
    {
        flush();
        m_cursorY += Constants::VSTEP;
    }
}
void BlockLayout::processWord(const QString& word)
{
    if (word.isEmpty())
        return;
    
    QFont font = getFont(m_size, m_weight, m_style);
    QFontMetrics metrics(font);
    auto wordWidth = metrics.horizontalAdvance(word);

    if (m_cursorX + wordWidth > width)
    {
        flush();
    }

    m_line.push_back({m_cursorX, word, font});

    m_cursorX += wordWidth + metrics.horizontalAdvance(' '); 
}

/// Align the words in a line along the baseline and add them to the display list.
void BlockLayout::flush()
{
    if (m_line.empty())
    {
        return;
    }
   
    auto [maxAscent, maxDescent] = getMaxAscentAndDescentForLine(); 
    int baseline = m_cursorY + 1.25 * maxAscent;

    for (const auto& item : m_line)
    {
        auto [itemX, word, font] = item;
        QFontMetrics metrics(font);
        int posX = x + itemX;
        int posY = y + baseline - metrics.ascent();
        Point point(posX, posY);
        m_displayList[point] = {word, font};
    }

    m_cursorY = baseline + 1.25 * maxDescent;
    m_cursorX = 0;
    m_line.clear();
}

std::tuple<int, int> BlockLayout::getMaxAscentAndDescentForLine() const
{
    int maxAscent = 0;
    int maxDescent = 0;

    for (const auto& item : m_line)
    {
        QFont font = std::get<2>(item);
        QFontMetrics metrics(font);
        if (metrics.ascent() > maxAscent)
        {
            maxAscent = metrics.ascent();
        }
        if (metrics.descent() > maxDescent)
        {
            maxDescent = metrics.descent();
        }
    }

    return {maxAscent, maxDescent};
}

std::vector<std::unique_ptr<Command>> BlockLayout::paint() const
{
    std::vector<std::unique_ptr<Command>> commands;

    if (auto element = dynamic_cast<const Element*>(node)) 
    {
        if (element->tag == "pre")
        {
            int x2 = x + width;
            int y2 = y + height;
            auto command = std::make_unique<DrawRect>(x, y, x2, y2, "gray");
            commands.push_back(std::move(command));
        }
    }

    if (getLayoutMode() == INLINE)
    {
        for (const auto& item : m_displayList)
        {
            Point point = item.first;
            QString text = std::get<0>(item.second);
            QFont font = std::get<1>(item.second);
            auto command = std::make_unique<DrawText>(point.x, point.y, text, font);
            commands.push_back(std::move(command));
        }
    }

    return commands;
}

QFont BlockLayout::getFont(int size, int weight, int style) 
{
    std::tuple<int, int, int> key{size, weight, style};
    if (!m_fonts.contains(key))
    {
        QFont font(m_family, size, weight, style);
        m_fonts[key] = font;
    }
    return m_fonts[key];
}