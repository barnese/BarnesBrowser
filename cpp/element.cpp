#include "element.h"

Element::Element(std::string text, bool isTag)
{
    m_text = text;
    m_isTag = isTag;
}

std::string Element::getText() const
{
    return m_text;
}

bool Element::isTag() const
{
    return m_isTag;
}
