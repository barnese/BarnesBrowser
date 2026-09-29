#include "document_layout.h"

#include "commands/transform.h"
#include "block_layout.h"
#include "constants.h"
#include "layout_helpers.h"
#include "frame.h"
#include "html_tree/node_helpers.h"

#include <sstream>
#include <iostream>

DocumentLayout::DocumentLayout(Node* node, Frame* frame)
{
    this->node = node;
    this->node->layoutObject = this;
    this->parent = nullptr;
    this->frame = frame;
    x = 0;
    y = 0;
    height = 0;
    hasDirtyDescendants = true;
}

void DocumentLayout::layout()
{
    throw DocumentLayoutException("layout() called without parameters!");
}

bool DocumentLayout::layoutNeeded() const
{
    if (zoom.isDirty()) return true;
    if (width.isDirty()) return true;
    if (hasDirtyDescendants) return true;
    return false;
}

void DocumentLayout::layout(int width, double zoom)
{
    if (!layoutNeeded()) return;

    this->zoom.set(zoom);
    this->width.set(width - 2 * dpx(Constants::HSTEP, zoom));

    std::unique_ptr<Layout> child;
    if (children.isEmpty())
        child = std::make_unique<BlockLayout>(node, this, nullptr, frame);
    else
        child = std::move(children.getValue()[0]);
    auto* childPtr = child.get();
    
    if (children.isEmpty())
        children.getValue().push_back(std::move(child)); 
    else
        children.getValue()[0] = std::move(child);
   
    x = dpx(Constants::HSTEP, zoom);
    y = dpx(Constants::VSTEP, zoom);

    childPtr->layout();
    hasDirtyDescendants = false;

    height = childPtr->height;

    // Assert that the layout is clean.
    auto list = treeToList<Layout>((Layout*)this);
    for (const auto& item : list)
    {
        assert(!item->layoutNeeded());
    }
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
        QRect rect = getRect();
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
      << x << ", " << y << ", " << width.get() << ", " << height
      << "}, node: " << node->describe() << ")";
    return s.str();
}
