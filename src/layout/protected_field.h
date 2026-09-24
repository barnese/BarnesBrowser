#pragma once

#include <cassert>
#include <utility>

/// Associates a dirty flag with its value.
template<typename ValueType>
class ProtectedField
{
public:
    /// Asserts the value is not dirty before returning it,
    /// which defensively helps you catch coding errors earlier.
    const ValueType& get() const
    {
        assert(!m_dirty);
        return m_value;
    }

    ValueType& get() 
    {
        assert(!m_dirty);
        return m_value;
    }

    /// Returns raw value without dirty checks.
    const ValueType& getValue() const { return m_value; }
    ValueType& getValue() { return m_value; }

    /// Sets the value and resets the dirty flag.
    void set(ValueType value)
    {
        m_value = std::move(value);
        m_dirty = false;
    }

    /// Sets this protected field as dirty.
    void mark() 
    {
        if (m_dirty) return;
        m_dirty = true;
    }

    bool isDirty() const
    {
        return m_dirty;
    }

private:
    ValueType m_value;
    bool m_dirty = true;
};
