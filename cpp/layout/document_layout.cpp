#include "document_layout.h"
#include "block_layout.h"
#include "../constants.h"
#include "layout.h"

DocumentLayout::DocumentLayout(const Node* node)
{
    this->node = node;
    x = 0;
    y = 0;
    width = 0;
    height = 0;
}

void DocumentLayout::layout()
{
    auto child = std::make_unique<BlockLayout>(node, this);
    auto* childPtr = child.get();
    children.push_back(std::move(child));
   
    width = Constants::WIDTH - 2 * Constants::HSTEP;
    x = Constants::HSTEP;
    y = Constants::VSTEP;

    childPtr->layout();
    height = childPtr->height;
}

DisplayList DocumentLayout::paint() const
{
    return {};
}
