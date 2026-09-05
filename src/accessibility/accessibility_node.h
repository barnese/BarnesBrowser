#pragma once

#include "html_tree/node.h"

#include <memory>
#include <string>
#include <QRect>
#include <vector>

/// Defines a node for an accessibility tree.
class AccessibilityNode
{
public:
    AccessibilityNode(const Node* node);
    void build();
    std::string describe() const;
    void printTree();
    const Node* getNode() { return m_node; }
    bool containsPoint(int x, int y);
    AccessibilityNode* hitTest(int x, int y);

    std::string role;
    std::string text;
    std::vector<std::unique_ptr<AccessibilityNode>> children;
    std::vector<QRect> bounds;

private:
    std::vector<QRect> computeBounds();
    void printTree(const AccessibilityNode* node, int depth = 0);
    void buildInternal(const Node* node);

    const Node* m_node;
};
