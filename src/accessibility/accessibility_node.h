#pragma once

#include "html_tree/node.h"

#include <memory>
#include <string>
#include <QRectF>
#include <vector>

class IAccessibilityNode
{
public:
    virtual ~IAccessibilityNode() = default;
};

/// Defines a node for an accessibility tree.
class AccessibilityNode : public IAccessibilityNode
{
public:
    AccessibilityNode(const Node* node, AccessibilityNode* parent = nullptr);
    void build();
    std::string describe() const;
    void printTree();
    const Node* getNode() { return m_node; }
    AccessibilityNode* getParent() { return m_parent; }
    bool containsPoint(int x, int y);
    AccessibilityNode* hitTest(int x, int y);
    std::vector<QRectF> absoluteBounds();
    void mapToParent(QRectF& rect);

    std::string role;
    std::string text;
    std::vector<std::unique_ptr<AccessibilityNode>> children;
    std::vector<QRectF> bounds;

private:
    std::vector<QRectF> computeBounds();
    void printTree(const AccessibilityNode* node, int depth = 0);
    void buildInternal(const Node* node);

    const Node* m_node;
    AccessibilityNode* m_parent;
};
