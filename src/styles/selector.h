#pragma once

#include "html_tree/node.h"

/// Abstract base class representing a CSS selector.
class Selector 
{
public:
    virtual ~Selector() = default;
    virtual bool matches(const Node* node) const = 0;
    virtual std::string getName() const = 0;
    virtual int getPriority() const = 0;
};
