#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

/// Abstract base class that represents a node of a tree.
class Node
{
public:
    virtual ~Node() = default;
    virtual std::string describe() const = 0;

    const std::vector<std::unique_ptr<Node>>& getChildren() const
    {
        return children;
    }

    void addChild(std::unique_ptr<Node> child)
    {
        child->parent = this;
        children.push_back(std::move(child));
    }

    std::string findStyle(const std::string& styleName, const std::string& defaultValue) const   
    {
        auto it = styles.find(styleName);
        return (it != styles.end()) ? it->second : defaultValue;
    }

    Node* parent = nullptr;
    std::vector<std::unique_ptr<Node>> children;
    std::map<std::string, std::string> styles;
};
