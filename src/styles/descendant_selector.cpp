#include "descendant_selector.h"

DescendantSelector::DescendantSelector(Selector* ancestor, Selector* descendant)
    : ancestor(ancestor),
      descendant(descendant)
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
