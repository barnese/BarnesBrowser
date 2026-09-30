#include "input_layout.h"

#include "commands/draw_line.h"
#include "commands/draw_rounded_rect.h"
#include "commands/draw_text.h"
#include "constants.h"
#include "helpers/string_helpers.h"
#include "html_tree/element.h"
#include "html_tree/text.h"
#include "layout/layout_effects_helpers.h"
#include "layout_helpers.h"

#include <QFontMetrics>
#include <memory>
#include <iostream>
#include <sstream>

InputLayout::InputLayout(
    Node* node, 
    Layout* parent, 
    Layout* previous,
    Frame* frame
) : EmbedLayout(node, parent, previous, frame)
{
}

void InputLayout::layout()
{
    if (!layoutNeeded()) return;
    EmbedLayout::layout();

    auto zoom = this->zoom.read(&width);
    width.set(dpx(Constants::INPUT_WIDTH_PX, zoom));

    QFontMetrics metrics(font.read(&height));
    height.set(metrics.lineSpacing());
    
    ascent.set(height.read(&ascent));
    descent.set(0);
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
        auto command = std::make_unique<DrawRoundedRect>(getRect(), bgColor, borderRadius);
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
                QFontMetrics metrics(font.get());
                auto cx = x.get() + metrics.horizontalAdvance(text) + 1;
                auto cy = y.get();
                auto command = std::make_unique<DrawLine>(cx, cy, cx, cy + height.get(), "black", 1);
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
        auto command = std::make_unique<DrawText>(x.get(), y.get(), text, font.get(), color);
        commands.push_back(std::move(command));
    }
    
    return commands;
}

std::vector<std::unique_ptr<Command>> InputLayout::paintEffects(std::vector<std::unique_ptr<Command>> commands) const
{
    auto zoom = this->zoom.get();
    auto rect = getRect();
    auto cmds = paintVisualEffects(node, std::move(commands), rect);
    paintOutline(node, cmds, rect, zoom);
    return cmds;
}

std::string InputLayout::describe() const
{
    std::stringstream s; 
    s << "InputLayout(rect: {"
      << x.get() << ", " << y.get() << ", " << width.get() << ", " << height.get()
      << "}, node: " << node->describe() << ")";
    return s.str();
}
