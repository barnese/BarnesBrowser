#pragma once

#include "node.h"
#include <string>
#include <map>

/// Represents a tag or "element" node in a tree.
class Element : public Node
{
public:
    Element(const std::string& tag, Node* parent = nullptr);
    std::string describe() const override;

    std::string tag;
    std::map<std::string, std::string> attributes;

    std::string findAttribute(const std::string& name, const std::string& defaultValue) const   
    {
        auto it = attributes.find(name);
        return (it != attributes.end()) ? it->second : defaultValue;
    }
};
