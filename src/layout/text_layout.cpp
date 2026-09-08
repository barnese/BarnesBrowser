#include "text_layout.h"

#include "commands/draw_text.h"
#include "styles/style_helpers.h"
#include "helpers/string_helpers.h"

#include <QFont>
#include <QFontMetrics>
#include <memory>
#include <sstream>

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
    this->node->layoutObject = this;
}

void TextLayout::layout()
{
    zoom = parent->zoom;
    font = getFont(parent->node, zoom);    
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

    auto colorStyle = node->findStyle("color", "black");
    auto color = QString::fromStdString(colorStyle);
    auto command = std::make_unique<DrawText>(x, y, word, font, color);
    commands.push_back(std::move(command));

    return commands;
}

std::vector<std::unique_ptr<Command>> TextLayout::paintEffects(std::vector<std::unique_ptr<Command>> commands) const
{
    return commands;
}

std::string TextLayout::describe() const
{
    std::stringstream s; 
    s << "TextLayout(rect: {"
      << x << ", " << y << ", " << width << ", " << height
      << "}, node: " << trim(node->describe()) << ")";
    return s.str();
}
