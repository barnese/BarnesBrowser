#pragma once

#include "node.h"
#include "element.h"

#include <optional>
#include <string>
#include <vector>

int getTabIndex(const Node* node);
bool isFocusable(const Node* node);
void dirtyStyle(Node* node);

/// Converts a tree to a vector.
/// @param node the root node of the tree
/// @param list reference to the list to populate
/// @returns vector of nodes in the tree
template<typename Tree>
void treeToList(Tree* node, std::vector<Tree*>& list)
{
    list.push_back(node);

    if constexpr (requires { node->children.getValue(); }) 
    {
        for (const auto& child : node->children.getValue())
            treeToList(child.get(), list);
    }
    else 
    {
        for (const auto& child : node->children)
            treeToList(child.get(), list);
    }
}

/// Convenience helper
template<typename Tree>
std::vector<Tree*> treeToList(Tree* node)
{
    std::vector<Tree*> list;
    treeToList(node, list);
    return list;
}

/// Converts a tree to a vector of elements.
/// @param node the root node of the tree
/// @param list reference to the list to populate
/// @param filterByTag optional - only add nodes with this tag name
/// @returns vector of elements in the tree
void treeToElementList(Node* node, std::vector<Element*>& list, std::optional<std::string> filterByTag = std::nullopt);

/// Convenience helper
std::vector<Element*> treeToElementList(Node* node, std::optional<std::string> filterByTag = std::nullopt);
