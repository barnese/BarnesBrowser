#pragma once

#include "layout.h"

#include <sstream>

template<typename T>
std::string ProtectedField<T>::describe()
{
    std::stringstream s;
    s << "ProtectedField(";
    if (m_layout && m_layout->node)
    {
        s << m_layout->node->describe();
    }
    else if (m_layout)
    {
        s << m_layout->describe();
    }
    s << ", ";
    s << m_name << ")";
    return s.str();
}

template<typename T>
void ProtectedField<T>::setAncestorDirtyFlags()
{
    auto parent = m_parent;
    while (parent && !parent->hasDirtyDescendants)
    {
        parent->hasDirtyDescendants = true;
        parent = parent->parent;
    }
}
