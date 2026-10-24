#include "frame_accessibility_node.h"

#include "html_tree/element.h"
#include "layout/layout_helpers.h"

FrameAccessibilityNode::FrameAccessibilityNode(const Node* node, AccessibilityNode* parent)
    : AccessibilityNode(node, parent)
{
    if (const auto element = dynamic_cast<const Element*>(node))
    {
        scroll = element->frame->scroll;
        zoom = element->layoutObject->zoom.get();
    }
    else 
    {
        scroll = 0;
        zoom = 1;
    }
}

AccessibilityNode* FrameAccessibilityNode::hitTest(int x, int y)
{
    // Use the frame's bounds to ignore clicks outside the frame bounds
    // and adjust clicks against the frame's coordinates.
    auto bounds = this->bounds[0];
    if (!bounds.contains(x, y)) return nullptr;
    auto newX = x - bounds.left() - dpx(1, zoom);
    auto newY = y - bounds.top() - dpx(1, zoom) + scroll;
    AccessibilityNode* node = this;
    for (const auto& child : children)
    {
        auto res = child->hitTest(newX, newY);
        if (res)
            node = res;
    }
    return node;
}

void FrameAccessibilityNode::mapToParent(QRectF& rect)
{
    auto bounds = this->bounds[0];
    rect.translate(bounds.left(), bounds.top() - scroll);
    rect.intersects(bounds);
}
