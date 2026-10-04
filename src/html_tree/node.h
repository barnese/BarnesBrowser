#pragma once

#include "layout/protected_field.h"
#include "styles/css_animation.h"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class Layout;
class VisualEffect;

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
        return (it != styles.end()) ? it->second.getValue() : defaultValue;
    }

    std::optional<std::string> findStyle(const std::string& styleName) const
    {
        auto it = styles.find(styleName);
        if (it == styles.end())
        {
            return std::nullopt;
        }
        return it->second.getValue();
    }

    Node* parent = nullptr;
    std::vector<std::unique_ptr<Node>> children;
    std::map<std::string, ProtectedField<std::string>> styles;
    std::map<std::string, std::unique_ptr<CSSAnimation>> animations;
    bool isFocused;
    VisualEffect* blendOp = nullptr;
    Layout* layoutObject;
};
