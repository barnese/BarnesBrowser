#include "block_layout.h"

#include "commands/draw_rounded_rect.h"
#include "constants.h"
#include "helpers/string_helpers.h"
#include "html_tree/element.h"
#include "html_tree/text.h"
#include "layout.h"
#include "layout/input_layout.h"
#include "layout/line_layout.h"
#include "layout/layout_effects_helpers.h"
#include "styles/style_helpers.h"
#include "text_layout.h"

#include <QFontMetrics>
#include <QRect>
#include <memory>

BlockLayout::BlockLayout(Node* node, const Layout* parent, const Layout* previous)
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

    height = 0;
    for (const auto& child : children)
        height += child->height;
}

void BlockLayout::layoutBlock()
{
    previous = nullptr;
    for (const auto& child : node->getChildren())
    {
        // Skip laying out the head tag.
        if (auto element = dynamic_cast<const Element*>(child.get())) 
        {
            if (element->tag == "head") 
            {
                continue;
            }
        }

        auto next = std::make_unique<BlockLayout>(child.get(), this, previous);
        previous = next.get();
        children.push_back(std::move(next));
    }
}

void BlockLayout::layoutText() 
{
    m_cursorX = 0;
    newLine();
    recurse(node);
}

/// Determines if any children contain block elements.
bool BlockLayout::areAnyChildBlockElements() const
{
    for (const auto& child : node->getChildren())
    {
        if (auto element = dynamic_cast<const Element*>(child.get()))
        {
            if (std::ranges::contains(Constants::BLOCK_ELEMENTS, element->tag))
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
    else if (auto element = dynamic_cast<const Element*>(node))
    {
        if (element->tag == "input") 
        {
            return INLINE;
        }
    }

    return BLOCK;
}

void BlockLayout::recurse(Node* node) 
{
    if (auto element = dynamic_cast<const Element*>(node)) 
    {
        if (element->tag == "br")
        {
            newLine();
        }
        else if (element->tag == "input" || element->tag == "button")
        {
            processInput(node);
        }
        else 
        {
            for (const auto& child : element->getChildren())
            {
                recurse(child.get());
            }
        }
    }
    else if (auto text = dynamic_cast<const Text*>(node))
    {
        QString content(text->getText().c_str());
        auto words = content.split(' ');

        for (auto& word : words)
        {
            processWord(node, word);
        }
    }
}

void BlockLayout::processInput(Node* node)
{
    const auto inputWidth = Constants::INPUT_WIDTH_PX;
    if (m_cursorX + inputWidth > width)
    {
        newLine();
    }

    auto line = children.back().get();
    auto previousWord = (line->children.empty()) ? nullptr : line->children.back().get();
    auto input = std::make_unique<InputLayout>(node, line, previousWord);
    line->children.push_back(std::move(input));

    const auto familyString = node->findStyle("font-family", "Helvetica");
    const auto family = QString::fromStdString(familyString);

    const auto weightString = node->findStyle("font-weight", "normal");
    const auto weight = cssFontWeightToQt(weightString);

    const auto styleString = node->findStyle("font-style", "normal");
    const auto style = cssFontStyleToQt(styleString);

    const auto sizeString = node->findStyle("font-size", "14px");
    const auto size = static_cast<int>(pixelsToFloat(sizeString));

    const QFont font = getFont(family, size, weight, style);
    const QFontMetrics metrics(font);
    const auto spaceWidth = metrics.horizontalAdvance(" ");

    m_cursorX += inputWidth + spaceWidth;
}

void BlockLayout::processWord(Node* node, QString& word)
{
    if (word.isEmpty())
        return;
    
    replaceHTMLCharacterEntities(word);

    const auto familyString = node->findStyle("font-family", "Helvetica");
    const auto family = QString::fromStdString(familyString);
    
    auto weightString = node->findStyle("font-weight", "normal");
    auto weight = cssFontWeightToQt(weightString);

    auto styleString = node->findStyle("font-style", "normal");
    auto style = cssFontStyleToQt(styleString);

    auto sizeString = node->findStyle("font-size", "14px");
    auto size = static_cast<int>(pixelsToFloat(sizeString));

    auto colorString = node->findStyle("color", "black");
    QString color = QString::fromStdString(colorString);

    QFont font = getFont(family, size, weight, style);
    QFontMetrics metrics(font);
    auto wordWidth = metrics.horizontalAdvance(word);

    if (m_cursorX + wordWidth > width)
    {
        newLine();
    }

    // Add the word as a TextLayout to the end of the current line.
    auto line = children.back().get();
    auto previousWord = (line->children.empty()) ? nullptr : line->children.back().get();
    auto text = std::make_unique<TextLayout>(node, word, line, previousWord);
    line->children.push_back(std::move(text));

    m_cursorX += wordWidth + metrics.horizontalAdvance(' '); 
}

/// Creates a new line layout and reset some fields.
void BlockLayout::newLine()
{
    m_cursorX = 0;
    auto lastLine = (children.empty()) ? nullptr : children.back().get();
    auto newLine = std::make_unique<LineLayout>(node, this, lastLine);
    children.push_back(std::move(newLine));
}

bool BlockLayout::shouldPaint() const
{
    if (dynamic_cast<const Text*>(node))
    {
        return true;
    }

    if (auto element = dynamic_cast<const Element*>(node))
    {
        if (element->tag != "input" && element->tag != "button")
        {
            return true;
        }
    }

    return false;
}

std::vector<std::unique_ptr<Command>> BlockLayout::paint() const
{
    std::vector<std::unique_ptr<Command>> commands;

    if (auto element = dynamic_cast<const Element*>(node)) 
    {
        if (std::ranges::contains(Constants::BLOCK_ELEMENTS, element->tag))
        {
            auto backgroundColor = element->findStyle("background-color", "transparent");
            // TODO: How should we handle background styles with multiple attributes?
            // For now, I'm treating it as an alias to background-color.
            if (backgroundColor == "transparent")
            {
                backgroundColor = element->findStyle("background", "transparent");
            }
            QString color(backgroundColor.c_str());
            auto borderRadius = element->findStyle("border-radius", "0px");
            auto radius = pixelsToFloat(borderRadius);
            auto command = std::make_unique<DrawRoundedRect>(x, y, width, height, color, radius);
            commands.push_back(std::move(command));
        }
    }

    return commands;
}

std::vector<std::unique_ptr<Command>> BlockLayout::paintEffects(std::vector<std::unique_ptr<Command>> commands) const
{
    QRect rect(x, y, width, height);
    return paintVisualEffects(node, std::move(commands), rect);
}

QFont BlockLayout::getFont(QString family, int size, int weight, int style) 
{
    std::tuple<QString, int, int, int> key{family, size, weight, style};
    if (!m_fonts.contains(key))
    {
        QFont font(family, size, weight, style);
        m_fonts[key] = font;
    }
    return m_fonts[key];
}
