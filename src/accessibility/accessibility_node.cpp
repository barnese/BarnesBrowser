#include "accessibility_node.h"

#include "html_tree/node_helpers.h"
#include "html_tree/element.h"
#include "html_tree/text.h"

#include <iostream>

AccessibilityNode::AccessibilityNode(const Node* node)
    : m_node(node)
{
    if (dynamic_cast<const Text*>(node))
    {
        if (isFocusable(node->parent))
        {
            role = "focusable text";
        }
        else 
        {
            role = "StaticText";
        }
    }
    else if (const auto element = dynamic_cast<const Element*>(node))
    {
        auto attrRole = element->findAttribute("role", "");

        if (!attrRole.empty())
        {
            role = attrRole;
        }
        else if (element->tag == "a")
        {
            role = "link";
        }
        else if (element->tag == "input")
        {
            role = "textbox";
        }
        else if (element->tag == "button")
        {
            role = "button";
        }
        else if (element->tag == "html")
        {
            role = "document";
        }
        else if (isFocusable(node))
        {
            role = "focusable";
        }
        else 
        {
            role = "none";
        }
    }
}

void AccessibilityNode::build()
{
    for (const auto& child : m_node->children)
    {
        buildInternal(child.get());
    }
    if (role == "StaticText") 
        text = m_node->describe();
    else if (role == "focusable text") 
        text = "Focusable text: " + m_node->describe();
    else if (role == "focusable")
        text = "Focusable element";
    else if (role == "textbox")
    {
        std::string value;        
        if (const auto element = dynamic_cast<const Element*>(m_node))
        {
            auto attr = element->findAttribute("value", "");
            if (!attr.empty())
            {
                value = attr;
            }
            else if (element->tag != "input" && !m_node->children.empty() && 
                dynamic_cast<const Text*>(m_node->children[0].get()))
            {
                value = m_node->children[0]->describe();
            }
            else 
            {
                value = "";
            }
        }
        text = "Input box: " + value;
    }
    else if (role == "button")
        text = "Button";
    else if (role == "link")
        text = "Link";
    else if (role == "alert")
        text = "Alert";
    else if (role == "document")
        text = "Document";

    if (m_node->isFocused)
        text += " is focused";
}

void AccessibilityNode::buildInternal(const Node* node)
{
    auto child = std::make_unique<AccessibilityNode>(node);
    if (child->role != "none")
    {
        auto childPtr = child.get();
        children.push_back(std::move(child));
        childPtr->build();
    }
    else
    {
        for (const auto& grandChildNode : node->children)
        {
            buildInternal(grandChildNode.get());
        }
    }
}

void AccessibilityNode::printTree()
{
    printTree(this, 0);
}

void AccessibilityNode::printTree(const AccessibilityNode* node, int depth)
{
    if (depth == 0)
        std::cout << std::endl << "Accessibility Tree:" << std::endl;
    std::string indent(depth * 2, ' ');
    std::cout << indent << node->describe() << std::endl;
    for (const auto& child : node->children)
        printTree(child.get(), depth + 1);
}

std::string AccessibilityNode::describe() const
{
    return "role = " + role + " text=" + text;
}
