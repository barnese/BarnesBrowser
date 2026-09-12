#include "block_layout.h"

#include "commands/draw_rounded_rect.h"
#include "constants.h"
#include "helpers/string_helpers.h"
#include "html_tree/element.h"
#include "html_tree/text.h"
#include "layout.h"
#include "layout/iframe_layout.h"
#include "layout/image_layout.h"
#include "layout/input_layout.h"
#include "layout/line_layout.h"
#include "layout/layout_effects_helpers.h"
#include "layout/layout_helpers.h"
#include "styles/style_helpers.h"
#include "text_layout.h"

#include <QFontMetrics>
#include <memory>
#include <sstream>

BlockLayout::BlockLayout(Node* node, const Layout* parent, const Layout* previous, Frame* frame)
{
    this->node = node;
    this->parent = parent;
    this->previous = previous;
    this->node->layoutObject = this;
    this->frame = frame;
    x = 0;
    y = 0;
    width = 0;
    height = 0;
}

/// Contructs the layout tree for the current tree node.
void BlockLayout::layout()
{
    zoom = parent->zoom;
    x = parent->x;
    width = overrideSizeWithStyle(node, parent->width, "width");

    if (previous)
        y = previous->y + previous->height;
    else 
        y = parent->y;

    auto mode = getLayoutMode();
    if (mode == BLOCK)
        layoutBlock();
    else 
        layoutText();

    height = 0;
    for (const auto& child : children)
    {
        child->layout();
        height += child->height;
    }

    height = overrideSizeWithStyle(node, height, "height");
}

void BlockLayout::layoutBlock()
{
    Layout* prev = nullptr;
    for (const auto& child : node->getChildren())
    {
        // Skip laying out the head and style tags.
        if (auto element = dynamic_cast<const Element*>(child.get())) 
        {
            if (element->tag == "head" || element->tag == "style") 
            {
                continue;
            }
        }

        auto next = std::make_unique<BlockLayout>(child.get(), this, prev, frame);
        prev = next.get();
        children.push_back(std::move(next));
    }
}

void BlockLayout::layoutText() 
{
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
    else if (!node->children.empty() && !areAnyChildBlockElements())
    {
        return INLINE;
    }
    else if (auto element = dynamic_cast<const Element*>(node))
    {
        if (element->tag == "input" || element->tag == "img" || element->tag == "iframe")
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
            input(node);
        }
        else if (element->tag == "img")
        {
            image(node);
        }
        else if (element->tag == "iframe" && element->attributes.contains("src"))
        {
            iframe(node);
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

        for (auto& w : words)
        {
            word(node, w);
        }
    }
}

void BlockLayout::iframe(Node* node)
{
    if (const auto& element = dynamic_cast<const Element*>(node))
    {
        int w = 0;
        auto width = element->findAttribute("width");
        if (width.has_value())
        {
            w = dpx(std::stoi(width.value()), zoom);
        }
        else 
        {
            w = Constants::IFRAME_WIDTH_PX + dpx(2, zoom);
        }
        addInlineChild<IFrameLayout>(node, w, frame);
    } 
}

void BlockLayout::image(Node* node)
{
    if (auto element = dynamic_cast<const Element*>(node))
    {
        int w = 0;
        auto widthAttr = element->findAttribute("width");
        if (widthAttr.has_value())
            w = dpx(std::stoi(widthAttr.value()), zoom);
        else
            w = dpx(element->image->width(), zoom);
        addInlineChild<ImageLayout>(node, w, frame);
    }
}

void BlockLayout::input(Node* node)
{
    auto w = dpx(Constants::INPUT_WIDTH_PX, zoom);
    addInlineChild<InputLayout>(node, w, frame);
}

void BlockLayout::word(Node* node, QString& word)
{
    replaceHTMLCharacterEntities(word);

    const QFont font = getFont(node, zoom);
    const QFontMetrics metrics(font);
    const auto wordWidth = metrics.horizontalAdvance(word);
    
    addInlineChild<TextLayout>(node, wordWidth, frame, word);
}

/// Creates a new line layout and reset some fields.
void BlockLayout::newLine()
{
    m_cursorX = x;
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
        const std::array tags = {"input", "button", "img", "iframe"};

        if (!std::ranges::contains(tags, element->tag))
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
    auto cmds = paintVisualEffects(node, std::move(commands), rect);
    paintOutline(node, cmds, rect, zoom);
    return cmds;
}

std::string BlockLayout::describe() const
{
    std::stringstream s; 
    s << "BlockLayout(rect: {"
      << x << ", " << y << ", " << width << ", " << height
      << "}, node: " << node->describe() << ")";
    return s.str();
}
