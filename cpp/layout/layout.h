#pragma once

#include <memory>
#include <vector>
#include "../point.h"
#include <QGraphicsScene>
#include "../html_tree/node.h"

using DisplayList = std::map<Point, std::tuple<QString, QFont>>; 

/// Abstract base class that represents a layout.
class Layout
{
public:
    virtual ~Layout() = default;
    virtual void layout() = 0;
    virtual DisplayList paint() const = 0;

    void addChild(std::unique_ptr<Layout> child)
    {
        child->parent = this;
        children.push_back(std::move(child));
    }

    const Node* node;
    const Layout* parent;
    const Layout* previous;
    std::vector<std::unique_ptr<Layout>> children;

    // Size and position of layout objects.
    int x;
    int y;
    int width;
    int height;
};
