#include "descendant_selector.h"

DescendantSelector::DescendantSelector(
    std::unique_ptr<Selector> ancestor, 
    std::unique_ptr<Selector> descendant)
    : ancestor(std::move(ancestor)),
      descendant(std::move(descendant)),
      m_priority(this->ancestor->getPriority() + this->descendant->getPriority())
{
}

bool DescendantSelector::matches(const Node* node) const
{
    if (!descendant->matches(node)) 
    {
        return false;
    }
    while (node->parent)
    {
        if (ancestor->matches(node->parent))
        {
            return true;
        }
        node = node->parent;
    }
    return false;
}

std::string DescendantSelector::getName() const
{
    return ancestor->getName() + " " + descendant->getName();
}

int DescendantSelector::getPriority() const
{
    return m_priority;
}
