#include "element.h"

Element::Element(const std::string& tag, Node* myParent)
    : tag(tag)
{
    parent = myParent;
}

std::string Element::getTag() const
{
    return tag;
}

std::string Element::describe() const
{
    return "<" + tag + ">";
}
