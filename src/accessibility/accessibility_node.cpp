#include "accessibility_node.h"

#include "html_tree/node_helpers.h"
#include "html_tree/element.h"
#include "html_tree/text.h"
#include "layout/layout_helpers.h"
#include "frame.h"
#include "frame_accessibility_node.h"

#include <iostream>

AccessibilityNode::AccessibilityNode(const Node* node, AccessibilityNode* parent)
    : m_node(node),
      m_parent(parent)
{
    bounds = computeBounds();

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
        else if (element->tag == "iframe")
        {
            role = "iframe";
        }
        else if (element->tag == "img")
        {
            role = "image";
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

std::vector<QRectF> AccessibilityNode::computeBounds()
{
    if (m_node->layoutObject)
    {
        return {absoluteBoundsForObj(m_node->layoutObject)};
    }

    // If there is no layout object, we need to walk up the tree
    // to find the parent with a BlockLayout and union all text 
    // nodes in all LineLayouts that are children of the current
    // node. Text nodes don't have a layout object.
    if (dynamic_cast<const Text*>(m_node))
    {
        return {};
    }

    auto inLine = m_node->parent;
    std::vector<QRectF> bounds;
    
    while (!inLine->layoutObject)
    {
        inLine = inLine->parent;
    }

    for (const auto& line : inLine->layoutObject->children.getValue())
    {
        QRectF lineBounds;
        for (const auto& child : line->children.getValue())
        {
            if (child->node->parent == m_node)
            {
                lineBounds |= child->getRect();
            }
        }
        bounds.push_back(lineBounds);
    }

    return bounds;
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
    else if (role == "image")
    {
        if (const auto element = dynamic_cast<const Element*>(m_node)) 
        {
            auto attr = element->findAttribute("alt");
            if (attr.has_value())
            {
                text = "Image: " + attr.value();
            }
            else
            {
                text = "Image";
            }
        }
    }
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
    std::unique_ptr<AccessibilityNode> child;

    if (const auto element = dynamic_cast<const Element*>(node))
    {
        if (element->tag == "iframe" && element->frame && element->frame->isLoaded())
        {
            child = std::make_unique<FrameAccessibilityNode>(node, this);
        }
        else
        {
            child = std::make_unique<AccessibilityNode>(node, this);
        }
    }
    else 
    {
        child = std::make_unique<AccessibilityNode>(node);
    }

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

bool AccessibilityNode::containsPoint(int x, int y)
{
    for (const auto& bound : bounds)
    {
        if (bound.contains(x, y))
        {
            return true;
        }
    }

    return false;
}

AccessibilityNode* AccessibilityNode::hitTest(int x, int y)
{
    AccessibilityNode* node = nullptr;

    if (this->containsPoint(x, y))
    {
        node = this;
    }

    for (const auto& child : children)
    {
        auto result = child->hitTest(x, y);
        if (result)
        {
            node = result;
        }
    }

    return node;
}

std::vector<QRectF> AccessibilityNode::absoluteBounds()
{
    std::vector<QRectF> absBounds;

    for (const auto& bound : this->bounds)
    {
        AccessibilityNode* obj;
        auto absBound = bound.translated(0, 0);
        if (dynamic_cast<FrameAccessibilityNode*>(this))
        {
            obj = m_parent;
        }
        else 
        {
            obj = this;
        }
        while (obj)
        {
            obj->mapToParent(absBound);
            obj = obj->getParent();
        }
    }

    return absBounds;
}

void AccessibilityNode::mapToParent(QRectF&)
{
    // Nothing to do.
}
