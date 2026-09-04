#include "element.h"

#include <sstream>

Element::Element(const std::string& tag, Node* myParent)
    : tag(tag)
{
    parent = myParent;
    isFocused = false;
}

std::string Element::describe() const
{
    std::stringstream s;

    s << "<" + tag;
    
    for (const auto& attr : attributes)
    {
        s << " " << attr.first << "=\"" << attr.second << "\"";
    }

    s << ">, styles: {";

    std::string separator = "";
    for (const auto& style : styles)
    {
        s << separator << style.first << ": " << style.second;
        separator = ", ";
    }

    s << "}";

    return s.str();
}
