#pragma once

#include "node.h"

int getTabIndex(const Node* node);
bool isFocusable(const Node* node);

/// Converts a tree to a vector.
template<typename Tree>
void treeToList(Tree* node, std::vector<Tree*>& list)
{
    list.push_back(node);
    for (const auto& child : node->children)
        treeToList(child.get(), list);
}

template<typename Tree>
std::vector<Tree*> treeToList(Tree* node)
{
    std::vector<Tree*> list;
    treeToList(node, list);
    return list;
}
