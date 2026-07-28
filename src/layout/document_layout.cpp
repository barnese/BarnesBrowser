#include "document_layout.h"

#include "block_layout.h"
#include "constants.h"

DocumentLayout::DocumentLayout(Node* node, int parentWidth)
{
    this->node = node;
    x = 0;
    y = 0;
    width = 0;
    height = 0;
    m_parentWidth = parentWidth;
}

void DocumentLayout::layout()
{
    auto child = std::make_unique<BlockLayout>(node, this);
    auto* childPtr = child.get();
    children.push_back(std::move(child));
   
    width = m_parentWidth - 2 * Constants::HSTEP;
    x = Constants::HSTEP;
    y = Constants::VSTEP;

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
