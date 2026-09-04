#pragma once

#include "selector.h"

#include <memory>
#include <string>

class PseudoClassSelector : public Selector
{
public:
    PseudoClassSelector(std::string pseudoClass, std::unique_ptr<Selector> base);
    bool matches(const Node* node) const override;
    std::string getName() const override;
    int getPriority() const override;

private:
    std::string m_pseudoClass;
    std::unique_ptr<Selector> m_base;
    int m_priority;
};
