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
        if (element->attributes.contains("tabindex")) 
            return true;
        else if (element->attributes.contains("contenteditable"))
            return true;

        std::vector<std::string> focusableElements = { "input", "button", "a" };
        return std::ranges::contains(focusableElements, element->tag);
    }

    return false;
}

void dirtyStyle(Node *node)
{
    node->styles.mark();
}

void treeToElementList(Node* node, std::vector<Element*>& list, std::optional<std::string> filterByTag)
{
    if (const auto& element = dynamic_cast<Element*>(node))
    {
        if (filterByTag.has_value())
        {
            if (element->tag == filterByTag)
            {
                list.push_back(element);
            }
        }
        else 
        {
            list.push_back(element);
        }
    }

    for (const auto& child : node->children)
    {
        treeToElementList(child.get(), list, filterByTag);
    }
}

std::vector<Element*> treeToElementList(Node* node, std::optional<std::string> filterByTag)
{
    std::vector<Element*> list;
    treeToElementList(node, list, filterByTag);
    return list;
}
