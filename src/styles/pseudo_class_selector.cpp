#include "pseudo_class_selector.h"

PseudoClassSelector::PseudoClassSelector(std::string pseudoClass, std::unique_ptr<Selector> base)
    : m_pseudoClass(pseudoClass),
      m_base(std::move(base)),
      m_priority(m_base->getPriority())
{
}

bool PseudoClassSelector::matches(const Node* node) const
{
    if (!m_base->matches(node)) 
        return false;

    if (m_pseudoClass == "focus")
        return node->isFocused;

    return false;
}

std::string PseudoClassSelector::getName() const
{
    return m_base->getName() + "." + m_pseudoClass;
}

int PseudoClassSelector::getPriority() const
{
    return m_priority;
}
