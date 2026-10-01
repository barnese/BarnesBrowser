#pragma once

#include "commands/command.h"
#include "html_tree/node.h"
#include "protected_field.h"

#include <memory>
#include <QFont>
#include <QRect>
#include <vector>

class Frame;

/// Abstract base class that represents a layout.
class Layout
{
public:
    virtual ~Layout() = default;
    virtual bool layoutNeeded() const = 0;
    virtual void layout() = 0;
    virtual std::vector<std::unique_ptr<Command>> paint() const = 0;
    virtual bool shouldPaint() const = 0;
    virtual std::vector<std::unique_ptr<Command>> paintEffects(std::vector<std::unique_ptr<Command>> commands) const = 0;
    virtual std::string describe() const = 0;

    QRect getRect() const
    {
        return QRect(x.get(), y.get(), width.get(), height.get());
    }

    Node* node = nullptr;
    Layout* parent = nullptr;
    Layout* previous = nullptr;
    Frame* frame = nullptr;
    
    // Children of this layout. This is a protected field,
    // which means it holds the children value and whether 
    // the value has changed as a means of handling invalidation.
    ProtectedField<std::vector<std::unique_ptr<Layout>>> children;

    // Size and position of layout objects.
    ProtectedField<double> zoom;
    ProtectedField<double> width;
    ProtectedField<double> height;
    ProtectedField<double> x;
    ProtectedField<double> y;

    ProtectedField<QFont> font;
    ProtectedField<int> ascent;
    ProtectedField<int> descent;

    // Tracks whether any descendant has a dirty protected field.
    bool hasDirtyDescendants = false;
};

// Because ProtectedField needs to use Layout, and because ProtectedField
// is a template class, we need to include the template implementation
// here to avoid circular includes. 
#include "protected_field.tpp" // IWYU pragma: keep
