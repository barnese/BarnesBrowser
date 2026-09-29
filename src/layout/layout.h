#pragma once

#include "commands/command.h"
#include "html_tree/node.h"
#include "protected_field.h"

#include <memory>
#include <QFont>
#include <QGraphicsScene>
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
        return QRect(x, y, width.get(), height);
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

    bool hasDirtyDescendants = false;

    int x;
    int y;
    double height;

    QFont font;
    int ascent;
    int descent;
};
