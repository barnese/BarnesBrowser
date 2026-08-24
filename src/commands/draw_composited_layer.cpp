#include "draw_composited_layer.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>

DrawCompositedLayer::DrawCompositedLayer(CompositedLayer* compositedLayer)
    : m_compositedLayer(compositedLayer)
{
    rect = m_compositedLayer->compositedBounds().toAlignedRect();
}

void DrawCompositedLayer::execute(PaintContext& context)
{
    auto* fbo = m_compositedLayer->getFramebuffer();

    if (!fbo || !fbo->isValid()) 
        return;

    auto bounds = m_compositedLayer->compositedBounds();
    auto painterTransform = context.painter.transform();
    auto targetRect = painterTransform.mapRect(bounds);
    auto opacity = static_cast<float>(context.painter.opacity());

    context.painter.beginNativePainting();

    // Configure OpenGL blending for transparent layers.
    auto* gl = QOpenGLContext::currentContext()->functions();
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    context.blitter->bind();
    context.blitter->setOpacity(opacity);

    auto viewport = QRect(QPoint(0, 0), context.viewportSize);
    auto targetTransform = QOpenGLTextureBlitter::targetTransform(targetRect, viewport);

    context.blitter->blit(fbo->texture(), targetTransform, QOpenGLTextureBlitter::OriginBottomLeft);
    context.blitter->release();
    context.painter.endNativePainting();
}

std::string DrawCompositedLayer::describe() const
{
    std::stringstream s; 
    s << "DrawCompositedLayer(layer: ";
    s << m_compositedLayer->displayItems->front()->describe();
    s << ")";
    return s.str();
}

std::unique_ptr<Command> DrawCompositedLayer::clone(std::unique_ptr<Command>)
{
    return nullptr;
}
