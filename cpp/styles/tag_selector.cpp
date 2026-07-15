#include "tag_selector.h"

#include "html_tree/element.h"

TagSelector::TagSelector(std::string tag)
    : m_tag(tag)
{   
}

bool TagSelector::matches(const Node* node) const
{
    if (auto element = dynamic_cast<const Element*>(node))
    {
        return element->tag == m_tag;
    }

    return false;
}
