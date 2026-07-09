#pragma once

#include <memory>
#include <vector>

/// Abstract base class that represents a node of a tree.
class Node
{
public:
    virtual ~Node() = default;

    Node* getParent() const
    {
        return parent;
    }

    const std::vector<std::unique_ptr<Node>>& getChildren() const
    {
        return children;
    }

    void addChild(std::unique_ptr<Node> child)
    {
        child->parent = this;
        children.push_back(std::move(child));
    }

private:
    Node* parent = nullptr;
    std::vector<std::unique_ptr<Node>> children;
};
