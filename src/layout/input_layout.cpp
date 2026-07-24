#include "input_layout.h"

#include "commands/draw_rect.h"
#include "commands/draw_text.h"
#include "constants.h"
#include "styles/style_helpers.h"
#include "helpers/string_helpers.h"
#include "html_tree/element.h"
#include "html_tree/text.h"

#include <QFontMetrics>
#include <memory>
#include <iostream>

InputLayout::InputLayout(
    const Node* node, 
    const Layout* parent, 
    const Layout* previous
)
{
    this->node = node;
    this->parent = parent;
    this->previous = previous;
    width = Constants::INPUT_WIDTH_PX;
}

void InputLayout::layout()
{
    auto weight = cssFontWeightToQt(node->findStyle("font-weight", "normal"));
    auto style = cssFontStyleToQt(node->findStyle("font-style", "normal"));
    auto size = static_cast<int>(pixelsToFloat(node->findStyle("font-size", "14px")));
    
    font = getFont(size, weight, style);
    
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

std::vector<std::unique_ptr<Command>> InputLayout::paint() const
{
    std::vector<std::unique_ptr<Command>> commands;

    auto bgColor = QString::fromStdString(node->findStyle("background-color", "transparent"));
    if (bgColor != "transparent")
    {
        auto command = std::make_unique<DrawRect>(x, y, width, height, bgColor);
        commands.push_back(std::move(command));
    }

    QString text;
    if (auto element = dynamic_cast<const Element*>(node))
    {
        if (element->tag == "input")
        {
            text = QString::fromStdString(element->findAttribute("value", ""));
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

            auto color = QString::fromStdString(node->findStyle("color", "black"));
            auto command = std::make_unique<DrawText>(x, y, text, font, color);
            commands.push_back(std::move(command));
        }
    }
    
    return commands;
}

QFont InputLayout::getFont(int size, int weight, int style) 
{
    std::tuple<int, int, int> key{size, weight, style};
    if (!m_fonts.contains(key))
    {
        QFont font("Helvetica", size, weight, style);
        m_fonts[key] = font;
    }
    return m_fonts[key];
}
