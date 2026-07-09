#include "element.h"

Element::Element(const std::string& tag)
    : tag(tag)
{
}

std::string Element::getTag() const
{
    return tag;
}
