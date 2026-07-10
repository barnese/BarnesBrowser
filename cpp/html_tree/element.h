#pragma once

#include <string>
#include "node.h"

/// Represents a tag or "element" node in a tree.
class Element : public Node
{
public:
    Element(const std::string& tag, Node* parent = nullptr);
    std::string getTag() const;
    std::string describe() const override;

private:
    std::string tag;
};
