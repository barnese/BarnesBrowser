#pragma once

#include "commands/command.h"
#include "html_tree/node.h"

#include <memory>
#include <QFont>
#include <QGraphicsScene>
#include <vector>

/// Abstract base class that represents a layout.
class Layout
{
public:
    virtual ~Layout() = default;
    virtual void layout() = 0;
    virtual std::vector<std::unique_ptr<Command>> paint() const = 0;
    virtual bool shouldPaint() const = 0;
    virtual std::vector<std::unique_ptr<Command>> paintEffects(std::vector<std::unique_ptr<Command>> commands) const = 0;
    virtual std::string describe() const = 0;

    void addChild(std::unique_ptr<Layout> child)
    {
        child->parent = this;
        children.push_back(std::move(child));
    }

    Node* node;
    const Layout* parent;
    const Layout* previous;
    std::vector<std::unique_ptr<Layout>> children;

    // Size and position of layout objects.
    int x;
    int y;
    int width;
    int height;
    QFont font;
};
