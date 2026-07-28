#include "element.h"

Element::Element(const std::string& tag, Node* myParent)
    : tag(tag)
{
    parent = myParent;
    isFocused = false;
}

std::string Element::describe() const
{
    std::string text = "<" + tag;
    
    for (const auto& attribute : attributes)
    {
        text += " ";
        text += attribute.first;
        text += "=\"";
        text += attribute.second;
        text += "\"";
    }

    text += ">";
    return text;
}
