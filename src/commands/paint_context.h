#pragma once

#include <QPainter>
#include <QOpenGLTextureBlitter>
#include <QSize>

struct PaintContext
{
    QPainter& painter;
    QOpenGLTextureBlitter* blitter;
    QSize viewportSize;
};
