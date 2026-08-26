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
    void raster();
    QOpenGLFramebufferObject* getFramebuffer() { return m_fbo.get(); }    
    QRectF compositedBounds() const;
    void add(Command* displayItem);
    
    /// Deteremines if display item has the same parents as the 
    /// existing ones in displayItems. This can be used to put
    /// paint commands into the same CompositedLayer if they have
    /// the same set of ancestor visual effects in the display list.
    bool canMerge(Command* displayItem);

    std::optional<std::vector<Command*>> displayItems;
private:
    std::unique_ptr<QOpenGLFramebufferObject> m_fbo;
};
