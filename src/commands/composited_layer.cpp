#include "composited_layer.h"

#include <QOpenGLPaintDevice>
#include <QOpenGLFunctions>

CompositedLayer::CompositedLayer(Command* displayItem)
{
    if (displayItem)
    {
        displayItems = std::vector<Command*>();
        displayItems->push_back(displayItem);
    }
    else
    {
        displayItems = std::nullopt;
    }
}

QRectF CompositedLayer::compositedBounds() const
{
    QRect rect;

    if (!displayItems.has_value()) 
        return rect;
    
    for (auto item : *displayItems)
    {
        // Join the item's rect with the local rect.
        rect |= item->rect;
    }

    // Add a little extra padding to the bounds by one on all sides.
    rect = rect.adjusted(-1, -1, 1, 1);

    return rect;
}

void CompositedLayer::raster()
{
    auto bounds = compositedBounds();
    auto size = bounds.toAlignedRect().size();

    if (size.isEmpty()) return;

    if (!m_fbo || m_fbo->size() != size)
    {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        m_fbo = std::make_unique<QOpenGLFramebufferObject>(size, format);
    }

    m_fbo->bind();
    
    QOpenGLPaintDevice device(size);
    QPainter painter(&device);
    PaintContext context { painter, nullptr, size };

    painter.setCompositionMode(QPainter::CompositionMode_Source);    
    painter.fillRect(QRect(QPoint(0, 0), size), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.save();
    painter.translate(-bounds.left(), -bounds.top());

    for (const auto& item : *displayItems)
    {
        item->execute(context);
    }

    painter.restore();
    
#if SHOW_COMPOSITED_LAYER_BORDERS
    {
        // For debugging: draws a red outline around composited layers.
        QPen pen(Qt::red);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        QRect debugRect(1, 1, size.width() - 2, size.height() - 2);
        painter.drawRect(debugRect);
    }
#endif

    painter.end();

    auto* gl = QOpenGLContext::currentContext()->functions();
    gl->glFlush();

    m_fbo->release();
}

void CompositedLayer::add(Command* displayItem)
{
    displayItems->push_back(displayItem);
}

bool CompositedLayer::canMerge(Command* displayItem)
{
    return displayItem->parent == displayItems->front()->parent;
}
