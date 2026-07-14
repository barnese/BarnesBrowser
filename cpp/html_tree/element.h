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
    std::map<std::string, std::string> styles;
};
