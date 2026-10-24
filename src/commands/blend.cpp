#include "blend.h"

#include <QPainter>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QOpenGLFunctions>
#include <iostream>
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
    needsCompositing = blendMode.has_value() || opacity < 1;

    // Expand the bounding box to enclose each child's rectangle.
    for (const auto& child : children)
    {
        rect = rect.united(child->rect);
    }

    m_rect = rect;
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

    context.painter.beginNativePainting();

    // Save the framebuffer and viewport we are currently drawing into.
    GLint previousFbo = 0;
    GLint previousViewport[4];
    gl->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFbo);
    gl->glGetIntegerv(GL_VIEWPORT, previousViewport);

    // Create an offscreen FBO.
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    format.setSamples(0);
    QSize rectSize(m_rect.size().width(), m_rect.size().height());
    auto fbo = QOpenGLFramebufferObject(rectSize, format);

    if (!fbo.isValid())
    {
        std::cerr << "[Blend] FBO is invalid" << std::endl;
        return;
    }

    if (!fbo.bind())
    {
        std::cerr << "[Blend] FBO bind failed" << std::endl;
        return;
    }

    {
        QOpenGLPaintDevice device(rectSize);
        QPainter layerPainter(&device);
        layerPainter.setCompositionMode(QPainter::CompositionMode_Source);
        layerPainter.fillRect(QRect(QPoint(0, 0), rectSize), Qt::transparent);
        layerPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        
        // Make child coordinates remain in the parent's coordinates.
        layerPainter.translate(-m_rect.left(), -m_rect.top());

        PaintContext blendContext { 
            layerPainter, 
            context.blitter,
            rectSize
        };
        
        for (const auto& item : children)
        {
            item->execute(blendContext);
        }
    } // layerPainter is destroyed here; FBO is still bound.
    
    QImage image = fbo.toImage();

    // Restore the destination framebuffer/viewport and composite
    // the completed FBO with the desired blend mode.
    gl->glBindFramebuffer(GL_FRAMEBUFFER, previousFbo);
    gl->glViewport(
        previousViewport[0],
        previousViewport[1],
        previousViewport[2],
        previousViewport[3]
    );

    context.painter.endNativePainting();
    context.painter.save();
    context.painter.setOpacity(m_opacity);

    if (m_blendMode.has_value())
    {
        context.painter.setCompositionMode(parseBlendMode(*m_blendMode));
    }

    // The code below drawImage attempts to move everything to the
    // GPU, and it mostly works, but it doesn't support all blending
    // modes. To do that, I think I need to use shaders. That's a
    // project for another day, so I'll keep the drawImage for now,
    // even though it uses the CPU.    
    context.painter.drawImage(m_rect.topLeft(), image);
    context.painter.restore();
}

// An attempt at full GPU drawing. This somewhat works, but has rendering issues.
/*
void Blend::execute(PaintContext& context)
{
    if (!m_blendMode.has_value() && m_opacity >= 1.0)
    {
        for (const auto& child : children)
            child->execute(context);

        return;
    }

    if (children.empty() || rect.isEmpty())
        return;

    QOpenGLContext* glContext = QOpenGLContext::currentContext();
    QOpenGLFunctions* gl = glContext->functions();

    // Save the parent framebuffer.
    GLint previousFbo = 0;
    gl->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFbo);

    GLint previousViewport[4] = {};
    gl->glGetIntegerv(GL_VIEWPORT, previousViewport);

    // Create an offscreen layer.
    QOpenGLFramebufferObjectFormat format;
    format.setSamples(0);
    format.setAttachment(QOpenGLFramebufferObject::NoAttachment);

    QOpenGLFramebufferObject layerFbo(rect.size(), format);

    if (!layerFbo.isValid())
        return;

    if (!layerFbo.bind())
        return;

    gl->glViewport(0, 0, layerFbo.width(), layerFbo.height());

    // Draw all children into the layer.
    {
        QOpenGLPaintDevice device(layerFbo.size());
        
        QPainter layerPainter(&device);
        layerPainter.setCompositionMode(QPainter::CompositionMode_Source);
        layerPainter.fillRect(QRect(QPoint(0, 0), layerFbo.size()), Qt::transparent);
        layerPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);

        // Children continue using coordinates from the parent canvas.
        layerPainter.translate(-rect.left(), -rect.top());

        PaintContext layerContext{
            layerPainter,
            context.blitter,
            layerFbo.size()
        };

        for (const auto& child : children)
            child->execute(layerContext);

        layerPainter.end();
    }

    // Restore the parent framebuffer.
    gl->glBindFramebuffer(
        GL_FRAMEBUFFER,
        static_cast<GLuint>(previousFbo));

    gl->glViewport(
        previousViewport[0],
        previousViewport[1],
        previousViewport[2],
        previousViewport[3]);


    context.painter.beginNativePainting();
    gl->glEnable(GL_BLEND);
    gl->glBlendEquation(GL_FUNC_ADD);

    if (m_blendMode.has_value())
    {
        if (*m_blendMode == "source")
        {    
            gl->glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
        }
        else if (*m_blendMode == "source-over")
        {            
            gl->glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
        else if (*m_blendMode == "destination-over")
        {
            gl->glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
        }
        else if (*m_blendMode == "destination")
        {
            gl->glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ZERO, GL_ONE);
        }
        else
        {
            gl->glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
    }
    else
    {
        gl->glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }

    // Composite the completed layer texture back onto the parent.
    QOpenGLTextureBlitter blitter;
    blitter.create();
    blitter.bind(GL_TEXTURE_2D);
    blitter.setOpacity(static_cast<float>(m_opacity));
    blitter.setRedBlueSwizzle(false);

    const QRect viewport(
        previousViewport[0],
        previousViewport[1],
        previousViewport[2],
        previousViewport[3]);

    const QMatrix4x4 targetTransform =
        QOpenGLTextureBlitter::targetTransform(
            QRectF(rect),
            viewport);

    blitter.blit(
        layerFbo.texture(),
        targetTransform,
        QOpenGLTextureBlitter::OriginBottomLeft);

    blitter.release();

    context.painter.endNativePainting();
}
*/

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

QRectF Blend::map(QRectF rect) const
{
    if (children.empty()) return rect;

    if (const auto& blend = dynamic_cast<Blend*>(children.back().get()))
    {
        if (blend->getBlendMode() == "destination-in")
        {
            QRectF bounds = rect.translated(0, 0);
            return bounds.intersected(blend->rect);
        }
    }

    return rect;
}
