#include "document_layout.h"

#include "commands/transform.h"
#include "block_layout.h"
#include "constants.h"
#include "layout_helpers.h"
#include "frame.h"

#include <sstream>
#include <iostream>

DocumentLayout::DocumentLayout(Node* node, int parentWidth)
{
    this->node = node;
    this->node->layoutObject = this;
    x = 0;
    y = 0;
    width = 0;
    height = 0;
    zoom = 1;
    m_parentWidth = parentWidth;
}

void DocumentLayout::layout()
{
    std::cout << "Warning, DocumentLayout::layout() called without zoom" << std::endl;
    layout(zoom);
}

void DocumentLayout::layout(double zoom)
{
    this->zoom = zoom;
    auto child = std::make_unique<BlockLayout>(node, this, nullptr, nullptr);
    auto* childPtr = child.get();
    children.push_back(std::move(child));
   
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
