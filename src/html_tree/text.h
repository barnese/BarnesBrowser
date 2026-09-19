#pragma once

#include "node.h"
#include <string>

/// Represents a text node in a tree.
class Text : public Node
{
public:
    Text(const std::string& text, Node* parent = nullptr);
    std::string getText() const;
    void appendText(std::string text);
    std::string describe() const override;

private:
    std::string text;
};
