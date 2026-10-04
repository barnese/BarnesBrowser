#pragma once

#include "html_tree/node.h"
#include "layout.h"

#include <sstream>

template<typename T>
std::string ProtectedField<T>::describe()
{
    std::stringstream s;
    s << "ProtectedField(";

    if (std::holds_alternative<Layout*>(m_obj))
    {
        Layout* layout = std::get<Layout*>(m_obj);
        if (layout && layout->node)
        {
            s << layout->node->describe();
        }
        else 
        {
            s << layout->describe();
        }
    }
    else if (std::holds_alternative<Node*>(m_obj))
    {
        Node* node = std::get<Node*>(m_obj);
        if (node)
        {
            s << node->describe();
        }
    }
    s << ", ";
    s << m_name << ")";
    return s.str();
}

template<typename T>
void ProtectedField<T>::setAncestorDirtyFlags()
{
    if (std::holds_alternative<Layout*>(m_parent))
    {
        Layout* parent = std::get<Layout*>(m_parent);
        while (parent && !parent->hasDirtyDescendants)
        {
            parent->hasDirtyDescendants = true;
            parent = parent->parent;
        }
    }
}
