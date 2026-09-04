#include "node_helpers.h"

#include "html_tree/element.h"

#include <algorithm>
#include <string>
#include <vector>

int getTabIndex(const Node* node)
{
    int tabIndex = 0;

    if (auto element = dynamic_cast<const Element*>(node))
    {
        tabIndex = std::stoi(element->findAttribute("tabindex", "9999999"));
    }

    return tabIndex == 0 ? 9999999 : tabIndex;
}

bool isFocusable(const Node* node)
{
    if (getTabIndex(node) < 0) return false;
    
    if (auto element = dynamic_cast<const Element*>(node))
    {
        if (element->attributes.contains("tabindex")) return true;
        std::vector<std::string> focusableElements = { "input", "button", "a" };
        return std::ranges::contains(focusableElements, element->tag);
    }

    return false;
}
