#pragma once

#include "accessibility_node.h"

class FrameAccessibilityNode : public AccessibilityNode
{
public:
    FrameAccessibilityNode(const Node* node, AccessibilityNode* parent = nullptr);
    AccessibilityNode* hitTest(int x, int y);
    void mapToParent(QRect& rect);

    int scroll;
    double zoom;
};
