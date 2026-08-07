#pragma once

#include <string>
#include <QImage>
#include <QRect>

/// Defines an interface for a graphics command for a browser scene.
class Command
{
public:
    virtual ~Command() = default;
    virtual void execute(QImage* surface, int scroll) = 0;
    virtual std::string describe() const = 0;

    QRect rect;
};
