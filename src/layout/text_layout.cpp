#include "text_layout.h"

#include "commands/draw_text.h"
#include "styles/style_helpers.h"
#include "helpers/string_helpers.h"

#include <QFontMetrics>
#include <memory>

TextLayout::TextLayout(
    Node* node, 
    QString word,
    const Layout* parent, 
    const Layout* previous
)
{
    this->node = node;
    this->word = word;
    this->parent = parent;
    this->previous = previous;
}

void TextLayout::layout()
{
    auto weight = cssFontWeightToQt(node->findStyle("font-weight", "normal"));
    auto style = cssFontStyleToQt(node->findStyle("font-style", "normal"));
    auto size = static_cast<int>(pixelsToFloat(node->findStyle("font-size", "14px")));
    
    font = getFont(size, weight, style);
    
    QFontMetrics metrics(font);
    width = metrics.horizontalAdvance(word);

    if (previous)
    {
        QFontMetrics previousMetrics(previous->font);
        auto space = previousMetrics.horizontalAdvance(" ");
        x = previous->x + space + previous->width;
    }
    else 
    {
        x = parent->x;
    }

    height = metrics.lineSpacing();
}

bool TextLayout::shouldPaint() const 
{
    return true;
}

std::vector<std::unique_ptr<Command>> TextLayout::paint() const
{
    std::vector<std::unique_ptr<Command>> commands;

    auto color = QString::fromStdString(node->findStyle("color", "black"));
    auto command = std::make_unique<DrawText>(x, y, word, font, color);
    commands.push_back(std::move(command));

    return commands;
}

QFont TextLayout::getFont(int size, int weight, int style) 
{
    std::tuple<int, int, int> key{size, weight, style};
    if (!m_fonts.contains(key))
    {
        QFont font("Helvetica", size, weight, style);
        m_fonts[key] = font;
    }
    return m_fonts[key];
}
