#pragma once

#include "command.h"

#include <QGraphicsItem>
#include <QOpenGLFramebufferObject>
#include <QPainter>
#include <QRect>
#include <optional>

class CompositedLayer
{
public:
    CompositedLayer(Command* displayItem);
    std::optional<std::vector<Command*>> displayItems;
    void raster();
    QOpenGLFramebufferObject* getFramebuffer() { return m_fbo.get(); }    
    QRectF compositedBounds() const;

private:
    std::unique_ptr<QOpenGLFramebufferObject> m_fbo;
};
