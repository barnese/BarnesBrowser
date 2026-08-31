#include "input_layout.h"

#include "commands/draw_line.h"
#include "commands/draw_rounded_rect.h"
#include "commands/draw_text.h"
#include "constants.h"
#include "styles/style_helpers.h"
#include "helpers/string_helpers.h"
#include "html_tree/element.h"
#include "html_tree/text.h"
#include "layout/layout_effects_helpers.h"
#include "layout/layout_helpers.h"

#include <QFontMetrics>
#include <memory>
#include <iostream>
#include <sstream>

InputLayout::InputLayout(
    Node* node, 
    const Layout* parent, 
    const Layout* previous
)
{
    this->node = node;
    this->parent = parent;
    this->previous = previous;
}

void InputLayout::layout()
{
    zoom = parent->zoom;
    width = Constants::INPUT_WIDTH_PX;

    auto family = QString::fromStdString(node->findStyle("font-family", "Helvetica"));
    auto weight = cssFontWeightToQt(node->findStyle("font-weight", "normal"));
    auto style = cssFontStyleToQt(node->findStyle("font-style", "normal"));
    auto sizePx = pixelsToFloat(node->findStyle("font-size", "14px"));
    auto size = dpx(sizePx * 0.75, zoom);
    font = getFont(family, size, weight, style);
    QFontMetrics metrics(font);

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

bool InputLayout::shouldPaint() const
{
    return true;
}

std::vector<std::unique_ptr<Command>> InputLayout::paint() const
{
    std::vector<std::unique_ptr<Command>> commands;

    auto bgColor = QString::fromStdString(node->findStyle("background-color", "transparent"));
    auto borderRadius = pixelsToFloat(node->findStyle("border-radius", "0px"));
    if (bgColor != "transparent")
    {
        auto command = std::make_unique<DrawRoundedRect>(x, y, width, height, bgColor, borderRadius);
        commands.push_back(std::move(command));
    }

    QString text;
    if (auto element = dynamic_cast<const Element*>(node))
    {
        if (element->tag == "input")
        {
            text = QString::fromStdString(element->findAttribute("value", ""));
            
            // Draw a cursor if this input element is focused.
            if (element->isFocused) 
            {
                QFontMetrics metrics(font);
                auto cx = x + metrics.horizontalAdvance(text) + 1;
                auto cy = y;
                auto command = std::make_unique<DrawLine>(cx, cy, cx, cy + height, "black", 1);
                commands.push_back(std::move(command));
            }
        }
        else if (element->tag == "button")
        {
            if (element->children.size() == 1)
            {
                if (auto t = dynamic_cast<const Text*>(element->children[0].get()))
                {
                    text = QString::fromStdString(t->getText());
                }
                else 
                {
                    text = "";
                }
            } 
            else
            {
                std::cout << "Ignoring HTML contents inside button!\n";
                text = "";
            }
        }

        auto color = QString::fromStdString(node->findStyle("color", "black"));
        auto command = std::make_unique<DrawText>(x, y, text, font, color);
        commands.push_back(std::move(command));
    }
    
    return commands;
}

QFont InputLayout::getFont(QString family, int size, int weight, int style) 
{
    std::tuple<QString, int, int, int> key{family, size, weight, style};
    if (!m_fonts.contains(key))
    {
        QFont font(family, size, weight, style);
        m_fonts[key] = font;
    }
    return m_fonts[key];
}

std::vector<std::unique_ptr<Command>> InputLayout::paintEffects(std::vector<std::unique_ptr<Command>> commands) const
{
    QRect rect(x, y, width, height);
    return paintVisualEffects(node, std::move(commands), rect);
}

std::string InputLayout::describe() const
{
    std::stringstream s; 
    s << "InputLayout(rect: {"
      << x << ", " << y << ", " << width << ", " << height
      << "}, node: " << node->describe() << ")";
    return s.str();
}
