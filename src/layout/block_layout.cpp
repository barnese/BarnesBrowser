#include "block_layout.h"

#include "commands/draw_line.h"
#include "commands/draw_rounded_rect.h"
#include "constants.h"
#include "helpers/string_helpers.h"
#include "html_tree/element.h"
#include "html_tree/node_helpers.h"
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

BlockLayout::BlockLayout(Node* node, Layout* parent, Layout* previous, Frame* frame)
{
    this->node = node;
    this->parent = parent;
    this->previous = previous;
    this->node->layoutObject = this;
    this->frame = frame;
    previousWord = nullptr;

    // Create the protected fields for block layouts. Note that
    // block layouts can freeze zoom, width, x and y dependencies.
    zoom.initialize(this, "zoom", parent, DepList{&parent->zoom});
    width.initialize(this, "width", parent, DepList{&parent->width});
    x.initialize(this, "x", parent, DepList{&parent->x});

    std::vector<Invalidatable*> yDependencies;
    if (this->previous)
    {
        yDependencies = {&previous->y, &previous->height};
    }
    else 
    {
        yDependencies.push_back(&parent->y);
    }
    y.initialize(this, "y", parent, yDependencies);

    children.initialize(this, "children", parent, std::nullopt, InvalidList{});
    
    // Height can be frozen once the children field is computed.
    height.initialize(this, "height", parent);

    hasDirtyDescendants = true;
}

bool BlockLayout::layoutNeeded() const
{
    if (zoom.isDirty()) return true;
    if (width.isDirty()) return true;
    if (height.isDirty()) return true;
    if (x.isDirty()) return true;
    if (y.isDirty()) return true;
    if (children.isDirty()) return true;
    if (hasDirtyDescendants) return true;
    return false;
}

/// Contructs the layout tree for the current tree node.
void BlockLayout::layout()
{
    if (!layoutNeeded()) return;

    zoom.copy(&parent->zoom);
    width.copy(&parent->width);
    x.copy(&parent->x);

    width.set(overrideSizeWithStyle(node, width.get(), "width"));

    double marginTop = 0;

    if (const auto element = dynamic_cast<const Element*>(node))
    {
        if (std::ranges::contains(Constants::BLOCK_ELEMENTS, element->tag))
        {
            auto marginTopStyle = node->findStyle("margin-top");

            if (marginTopStyle)
            {
                marginTop = pixelsToDouble(*marginTopStyle);
            }
        }
    }

    if (previous)
    {
        auto prevY = previous->y.read(&y);
        auto prevHeight = previous->height.read(&y);
        y.set(prevY + prevHeight + marginTop);
    }
    else 
    {
        auto yValue = parent->y.read(&y) + marginTop;
        y.set(yValue);
    }

    auto mode = getLayoutMode();
    if (mode == BLOCK)
        layoutBlock();
    else 
        layoutText();

    DepList heightDeps;
    for (const auto& child : this->children.get())
    {
        heightDeps.push_back(&child->height);
    }
    heightDeps.push_back(&children);
    height.setDependenciesAndFreeze(heightDeps);

    for (const auto& child : children.get())
    {
        child->layout();
    }

    hasDirtyDescendants = false;

    auto maxHeight = 0;
    for (const auto& child : children.read(&height))
    {        
        maxHeight += child->height.read(&height);
    }

    double marginBottom = 0;

    if (const auto element = dynamic_cast<const Element*>(node))
    {
        if (std::ranges::contains(Constants::BLOCK_ELEMENTS, element->tag))
        {
            auto marginBottomStyle = node->findStyle("margin-bottom");            

            if (marginBottomStyle)
            {
                marginBottom += pixelsToDouble(*marginBottomStyle);
            }
        }
    }

    maxHeight += marginTop + marginBottom;

    height.set(overrideSizeWithStyle(node, maxHeight, "height"));
}

void BlockLayout::layoutBlock()
{
    if (!children.isDirty()) return;

    // Fill a temporary vector of children that can be set
    // to the member variable at the end. Creating this
    // vector also ensures idempotency.
    std::vector<std::unique_ptr<Layout>> children;

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

    this->children.set(std::move(children));
}

void BlockLayout::layoutText() 
{
    if (!children.isDirty()) return;

    // Clear the children to ensure idempotency
    tempChildren.clear();
    newLine();
    recurse(node);
    children.set(std::move(tempChildren));
    tempChildren.clear();
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
            
            if (w.endsWith("\n"))
            {
                newLine();
            }
        }
    }
}

void BlockLayout::iframe(Node* node)
{
    auto zoom = this->zoom.read(&children);
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
    auto zoom = this->zoom.read(&children);
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
    auto zoom = this->zoom.read(&children);
    auto w = dpx(Constants::INPUT_WIDTH_PX, zoom);
    addInlineChild<InputLayout>(node, w, frame);
}

void BlockLayout::word(Node* node, QString& word)
{
    std::string text = word.toStdString();
    replaceEntitiesWithHTMLChars(text);
    word = QString::fromStdString(text);

    auto zoom = this->zoom.read(&children);

    const QFont font = getFont(node, zoom, 0, &children);
    const QFontMetrics metrics(font);
    const auto wordWidth = metrics.horizontalAdvance(word);
    
    addInlineChild<TextLayout>(node, wordWidth, frame, word);
}

/// Creates a new line layout and reset some fields.
void BlockLayout::newLine()
{
    previousWord = nullptr;
    m_cursorX = 0;
    auto lastLine = (tempChildren.empty()) ? nullptr : tempChildren.back().get();
    auto newLine = std::make_unique<LineLayout>(node, this, lastLine);
    tempChildren.push_back(std::move(newLine));
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
            auto radius = dpx(pixelsToFloat(borderRadius), zoom.get());
            auto command = std::make_unique<DrawRoundedRect>(getRect(), color, radius);
            commands.push_back(std::move(command));
        }

        if (element->isFocused && element->attributes.contains("contenteditable"))
        {
            auto list = treeToList<Layout>((Layout*)this);
            std::vector<TextLayout*> textLayouts;
            for (const auto& layout : list)
            {
                if (auto* text = dynamic_cast<TextLayout*>(layout))
                {
                    textLayouts.push_back(text);
                }
            }

            if (!textLayouts.empty())
            {
                auto layout = textLayouts.back();
                commands.push_back(drawCursor(layout, layout->width.get()));
            }
            else 
            {
                commands.push_back(drawCursor((Layout*)this, 0));
            }
        }
    }

    return commands;
}

std::vector<std::unique_ptr<Command>> BlockLayout::paintEffects(std::vector<std::unique_ptr<Command>> commands) const
{
    return paintVisualEffects(node, std::move(commands), getRect());
}

std::string BlockLayout::describe() const
{
    std::stringstream s; 
    s << "BlockLayout(rect: {"
      << x.get() << ", " << y.get() << ", " << width.get() << ", " << height.get()
      << "}, node: " << node->describe() << ")";
    return s.str();
}
