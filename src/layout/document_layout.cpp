#include "document_layout.h"

#include "commands/transform.h"
#include "block_layout.h"
#include "constants.h"
#include "layout_helpers.h"
#include "frame.h"

#include <sstream>
#include <iostream>

DocumentLayout::DocumentLayout(Node* node)
{
    this->node = node;
    this->node->layoutObject = this;
    x = 0;
    y = 0;
    width = 0;
    height = 0;
    zoom = 1;
}

void DocumentLayout::layout()
{
    throw DocumentLayoutException("layout() called without parameters!");
}

void DocumentLayout::layout(int parentWidth, double zoom)
{
    m_parentWidth = parentWidth;
    this->zoom = zoom;
    std::unique_ptr<Layout> child;
    if (children.empty())
        child = std::make_unique<BlockLayout>(node, this, nullptr, nullptr);
    else
        child = std::move(children[0]);
    auto* childPtr = child.get();

    // Ensure idempotency
    if (children.empty())
        children.push_back(std::move(child)); 
    else
        children[0] = std::move(child);
   
    width = m_parentWidth - 2 * dpx(Constants::HSTEP, zoom);
    x = dpx(Constants::HSTEP, zoom);
    y = dpx(Constants::VSTEP, zoom);

    childPtr->layout();
    height = childPtr->height;
}

bool DocumentLayout::shouldPaint() const 
{
    return true;
}

std::vector<std::unique_ptr<Command>> DocumentLayout::paint() const
{
    return {};
}

std::vector<std::unique_ptr<Command>> DocumentLayout::paintEffects(std::vector<std::unique_ptr<Command>> commands) const
{
    if (frame && frame != frame->getTabRootFrame() && frame->scroll != 0)
    {
        QRect rect(x, y, width, height);
        QPointF translation(0, -frame->scroll);
        auto transform = std::make_unique<Transform>(translation, rect, node, std::move(commands));
        commands = commandList(std::move(transform));
    }
    
    return commands;
}

std::string DocumentLayout::describe() const
{
    std::stringstream s; 
    s << "DocumentLayout(rect: {"
      << x << ", " << y << ", " << width << ", " << height
      << "}, node: " << node->describe() << ")";
    return s.str();
}
