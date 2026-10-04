#pragma once

#include <cassert>
#include <optional>
#include <set>
#include <utility>
#include <variant>
#include <vector>

class Node;
class Layout;

/// Abstraction for fields whose current state can be marked as invalid.
class Invalidatable
{
public:
    virtual ~Invalidatable() = default;
    virtual void mark() = 0;

    bool frozenDependencies = false;
    bool frozenInvalidations = false;
    std::set<Invalidatable*> invalidations = {};
};

using DepList = std::vector<Invalidatable*>;
using InvalidList = std::vector<Invalidatable*>;

/// Represents the ProtectedField's owning object which is either a
/// monostate (AKA None), a Node or a Layout.
using ProtectedFieldOwner = std::variant<std::monostate, Node*, Layout*>;

/// Represents a protected field along with its value and a dirty flag.
/// Each field can have a list of dependencies, creating a dependency graph.
/// If dependencies are provided to the constructor, we "freeze" them 
/// so that others can't be added dynamically. If no dependencies are given,
/// then we automatically add them during "reads". Initializing them in the
/// constructor is preferred, since it allows explicitly defining the 
/// dependency graph manually to avoid cycles and undesired long chains.
template<typename T>
class ProtectedField : public Invalidatable
{
public:
    ProtectedField() = default;

    // Explicitly delete copy/move to guardrail using protected fields.
    // ProtectedField(const ProtectedField&) = delete;
    // ProtectedField& operator=(const ProtectedField&) = delete;
    // ProtectedField(ProtectedField&&) = delete;
    // ProtectedField& operator=(ProtectedField&&) = delete;

    void initialize(
        ProtectedFieldOwner obj, 
        std::string name, 
        ProtectedFieldOwner parent = std::monostate{},
        std::optional<std::vector<Invalidatable*>> dependencies = std::nullopt,
        std::optional<std::vector<Invalidatable*>> invalidations = std::nullopt) 
    {
        m_obj = obj;
        m_parent = parent;
        m_name = name;

        // If dependencies are given, automatically freeze dependencies 
        // for this field so that others can't be added later via reads.
        if (dependencies.has_value())
        {
            frozenDependencies = true;
            for (auto& dependency : *dependencies)
            {
                dependency->invalidations.insert(this);
            }            
        }

        // Only for adding children invalidations.
        if (invalidations.has_value())
        {
            frozenInvalidations = true;
            assert(name == "children");
            for (auto& invalidation : *invalidations)
            {
                this->invalidations.insert(invalidation);
            }
        }
    }

    void setDependenciesAndFreeze(std::vector<Invalidatable*> dependencies)
    {
        for (const auto& dependency : dependencies)
        {        
            dependency->invalidations.insert(this);
        }

        frozenDependencies = true;
    }

    /// Sets this protected field as dirty.
    void mark() override
    {
        if (m_dirty) return;
        m_dirty = true;
        setAncestorDirtyFlags();
    }


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
        // If the notify field has frozen dependencies, we can 
        // "freeze" the protected field so that this reading doesn't
        // add new dependencies automatically. We just asset that they
        // were declared.
        if (notify->frozenDependencies || frozenInvalidations)        
        {
            //assert(invalidations.contains(notify));
        }
        else 
        {
            invalidations.insert(notify);
        }

        return getValue();
    }

    /// Sets the value and resets the dirty flag.
    void set(T value)
    {
        // Use this debugging statement to inspect which fields 
        // are getting recomputed.
        //std::cout << "Change " << describe() << std::endl;

        if (value != m_value)
            notify();

        m_value = std::move(value);
        m_dirty = false;
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
    /// Sets descendant flags by walking the parent chain.
    void setAncestorDirtyFlags();

    /// Marks each invalidation field as dirty, if the field is still alive.
    void notify()
    {
        for (auto& invalidation : this->invalidations)
        {
            invalidation->mark();
        }

        setAncestorDirtyFlags();
    }

    T m_value;
    bool m_dirty = true;
    ProtectedFieldOwner m_obj;
    ProtectedFieldOwner m_parent;
    std::string m_name;
};
