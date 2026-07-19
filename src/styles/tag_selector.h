#pragma once

#include "selector.h"

#include <string>

/// Represents a CSS tag selector.
/// For example, a p tag selects all <p> elements.
class TagSelector : public Selector
{
public:
    TagSelector(std::string tag);
    bool matches(const Node* node) const override;
    std::string getName() const override;

private:
    std::string m_tag;
};
