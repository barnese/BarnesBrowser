#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

class Layout;

/// Abstraction for fields whose current state can be marked as invalid.
class Invalidatable
{
public:
    virtual ~Invalidatable() = default;
    virtual void mark() = 0;

    std::weak_ptr<void> lifetime() const 
    { 
        return m_lifetime; 
    }

private:
    /// A shared token whose purpose is to indicate whether 
    /// this invalidatable field is still alive. The int is 
    /// just a dummy value. When the InvalidationField is 
    /// destroyed, this lifetime token will also be destroyed.
    std::shared_ptr<void> m_lifetime = std::make_shared<int>(0);
};

/// Represents an invalidation field that keeps track of 
/// its own lifetime to prevent accessing a dangling pointer 
/// to a invalidatable that has gone out of scope.
struct InvalidationField
{
    Invalidatable* field;
    std::weak_ptr<void> lifetime;
};

/// Represents a protected field along with its value and a dirty flag.
/// Automatically handles dependency tracking of dependent fields.
template<typename T>
class ProtectedField : public Invalidatable
{
public:
    ProtectedField() = default;

    ProtectedField(Layout* layout, std::string name) 
        : m_layout(layout), m_name(name) {}

    /// Asserts the value is not dirty before returning it,
    /// which defensively helps you catch coding errors earlier.
    const T& get() const
    {
        assert(!m_dirty);
        return m_value;
    }

    T& get() 
    {
        assert(!m_dirty);
        return m_value;
    }

    /// Returns raw value without dirty checks.
    const T& getValue() const { return m_value; }
    T& getValue() { return m_value; }

    /// A variant of get that automatically handles maintaining
    /// the list of invalidations.
    T& read(Invalidatable* notify)
    {
        m_invalidations.push_back({notify, notify->lifetime()});
        return get();
    }

    /// Sets the value and resets the dirty flag.
    void set(T value)
    {
        //std::cout << "Change " << describe() << std::endl;
        notify();
        m_value = std::move(value);
        m_dirty = false;
    }

    /// Sets this protected field as dirty.
    void mark() override
    {
        if (m_dirty) return;
        m_dirty = true;
    }

    bool isDirty() const
    {
        return m_dirty;
    }

    /// Copies the given field's value into this field.
    void copy(ProtectedField<T>* field)
    {
        set(field->read(this));
    }

    bool isEmpty() const
    {
        if constexpr (requires { m_value.empty(); }) {
            return m_value.empty();
        }

        return false;
    }

    std::string describe();

private:
    /// Marks each invalidation field as dirty, if the field is still alive.
    void notify()
    {
        for (auto& invalidation : m_invalidations)
        {
            if (!invalidation.lifetime.expired())
            {
                invalidation.field->mark();
            }
        }
    }

    T m_value;
    bool m_dirty = true;
    Layout* m_layout = nullptr;
    std::string m_name;

    /// Used to track fields that depend on this protected field.
    /// This can be any type, so we'll use a list of invalidation fields.
    std::vector<InvalidationField> m_invalidations;
};
