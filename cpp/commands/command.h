#pragma once

#include <QGraphicsScene>

/// Defines an interface for a graphics command for a browser scene.
class Command
{
public:
    virtual ~Command() = default;
    virtual void execute(QGraphicsScene* scene, int scroll) = 0;

    int left;
    int top;
    int right;
    int bottom;
};
