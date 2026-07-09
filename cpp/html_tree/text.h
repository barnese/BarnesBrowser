#pragma once

#include <string>
#include "node.h"

/// Represents a text node in a tree.
class Text : public Node
{
public:
    Text(const std::string& text);
    std::string getText() const;

private:
    std::string text;
};
