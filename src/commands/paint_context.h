#pragma once

#include <QPainter>
#include <QOpenGLTextureBlitter>
#include <QSize>

/// Context used to allow commands to paint their contents.
struct PaintContext
{
    QPainter& painter;
    QOpenGLTextureBlitter* blitter;
    QSize viewportSize;
    int scroll = 0;
};
