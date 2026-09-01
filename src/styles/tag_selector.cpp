#include "tag_selector.h"

#include "html_tree/element.h"
#include "constants.h"

TagSelector::TagSelector(std::string tag)
    : m_tag(tag)
{
    m_priority = 1;
}

bool TagSelector::matches(const Node* node) const
{
    if (auto element = dynamic_cast<const Element*>(node))
    {
        // Remove the media dark prefix, which is used to override
        // selectors for dark mode. This is somewhat of a hack.
        // See BrowerTab::addStyles() for more details.
        auto tag = m_tag;
        auto pos = tag.find(Constants::MEDIA_DARK_PREFIX);
        if (pos != std::string::npos)
            tag.erase(pos, Constants::MEDIA_DARK_PREFIX.length());

        return element->tag == tag;
    }

    return false;
}

std::string TagSelector::getName() const
{
    return m_tag;
}

int TagSelector::getPriority() const
{
    return m_priority;
}
