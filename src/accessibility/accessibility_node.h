#pragma once

#include "html_tree/node.h"

#include <memory>
#include <string>
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

    std::string role;
    std::string text;
    std::vector<std::unique_ptr<AccessibilityNode>> children;

private:
    void printTree(const AccessibilityNode* node, int depth = 0);
    void buildInternal(const Node* node);

    const Node* m_node;
};
