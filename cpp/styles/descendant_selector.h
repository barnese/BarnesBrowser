#pragma once

#include "selector.h"

class DescendantSelector : public Selector
{
public:
    DescendantSelector(Selector* ancestor, Selector* descendant);
    bool matches(const Node* node) const override;

    Selector* ancestor;
    Selector* descendant;
};
