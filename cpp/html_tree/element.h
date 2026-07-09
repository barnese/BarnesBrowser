#pragma once

#include <string>
#include "node.h"

/// Represents a tag or "element" node in a tree.
class Element : public Node
{
public:
    Element(const std::string& tag);
    std::string getTag() const;
private:
    std::string tag;
};
