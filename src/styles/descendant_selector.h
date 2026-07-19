#pragma once

#include "selector.h"
#include <memory>

class DescendantSelector : public Selector
{
public:
    DescendantSelector(
        std::unique_ptr<Selector> ancestor, 
        std::unique_ptr<Selector> descendant
    );

    bool matches(const Node* node) const override;
    std::string getName() const override;

    std::unique_ptr<Selector> ancestor;
    std::unique_ptr<Selector> descendant;
};
