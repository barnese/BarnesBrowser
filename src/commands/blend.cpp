#include "blend.h"

#include <QPainter>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QOpenGLFunctions>
#include <sstream>

Blend::Blend(
    std::optional<std::string> blendMode, 
    double opacity,
    Node* node,
    std::vector<std::unique_ptr<Command>> children
) : VisualEffect({}, std::move(children), node)
{
    m_blendMode = blendMode;
    m_opacity = opacity;

    // Expand the bounding box to enclose each child's rectangle.
    for (const auto& child : children)
    {
        rect = rect.united(child->rect);
    }
}

QPainter::CompositionMode parseBlendMode(const std::string& blendMode)
{
    if (blendMode == "multiply")
    {
        return QPainter::CompositionMode_Multiply;
    }
    else if (blendMode == "difference")
    {
        return QPainter::CompositionMode_Difference;
    }
    else if (blendMode == "destination-in")
    {
        return QPainter::CompositionMode_DestinationIn;
    }
    else if (blendMode == "source-over")
    {
        return QPainter::CompositionMode_SourceOver;
    }
    else 
    {
        return QPainter::CompositionMode_SourceOver;
    }
}

void Blend::execute(PaintContext& context)
{
    // If no blending is needed, just run the children and return.
    if (!m_blendMode.has_value() && m_opacity >= 1)
    {
        for (const auto& child : children)
        {
            child->execute(context);
        }
        return;
    }
    
    auto* glContext = QOpenGLContext::currentContext();
    auto* gl = glContext->functions();

    // Save the framebuffer we are currently drawing into.
    GLint previousFbo = 0;
    gl->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFbo);

    // Create an offscreen FBO.
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(0); // Non-multisampled
    auto fbo = QOpenGLFramebufferObject(rect.size(), format);

    // Draw the children onto that FBO.
    fbo.bind();

    QOpenGLPaintDevice device(rect.size());
    QPainter painter(&device);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(QRect(QPoint(0, 0), rect.size()), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.translate(-rect.left(), -rect.top());

    PaintContext blendContext { painter, context.blitter, rect.size() };

    for (const auto& item : children)
    {
        item->execute(blendContext);
    }

    // Restore the destination framebuffer and composite
    // the completed FBO with the desired blend mode.
    gl->glBindFramebuffer(GL_FRAMEBUFFER, previousFbo);
    
    context.painter.save();

    if (m_blendMode.has_value())
    {
        painter.setCompositionMode(parseBlendMode(m_blendMode.value()));
    }

    context.painter.setOpacity(m_opacity);
    
    // The code below drawImage attempts to move everything to the
    // GPU, and it mostly works, but it doesn't support all blending
    // modes. To do that, I think I need to use shaders. That's a
    // project for another day, so I'll keep the drawImage for now,
    // even though it uses the CPU.
    context.painter.drawImage(rect.topLeft(), fbo.toImage());

    // context.painter.beginNativePainting();

    // gl->glEnable(GL_BLEND);
    // gl->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    // context.blitter->bind();
    // context.blitter->setOpacity(m_opacity);

    // const QRect viewport(QPoint(0, 0), context.viewportSize);
    // const QMatrix4x4 target = QOpenGLTextureBlitter::targetTransform(rect, viewport);

    // context.blitter->blit(
    //     fbo.texture(),
    //     target,
    //     QOpenGLTextureBlitter::OriginBottomLeft);

    // context.blitter->setOpacity(1.0f);
    // context.blitter->release();

    // gl->glDisable(GL_BLEND);

    // context.painter.endNativePainting();

    context.painter.restore();
}

std::unique_ptr<Command> Blend::clone(std::unique_ptr<Command> child)
{
    std::vector<std::unique_ptr<Command>> children;
    children.push_back(std::move(child));
    return std::make_unique<Blend>(m_blendMode, m_opacity, node, std::move(children));
}

std::string Blend::describe() const
{    
    std::stringstream s; 
    s << "Blend(children: " << children.size();
    s << ", opacity: " << m_opacity;
    s << ", blend: " << (m_blendMode.has_value() ? m_blendMode.value() : "not set");
    s << ")";
    return s.str();
}
