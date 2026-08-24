#include "blend.h"

#include <QPainter>
#include <QGraphicsItemGroup>
#include <sstream>

Blend::Blend(
    std::optional<std::string> blendMode, 
    double opacity,
    std::vector<std::unique_ptr<Command>> children
) : VisualEffect({}, std::move(children))
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
    if (m_blendMode.has_value() || m_opacity < 1)
    {
        // Create an offscreen layer matching the destination.
        QImage layer(rect.size(), QImage::Format_RGBA8888);
        layer.fill(Qt::transparent);

        QPainter layerPainter(&layer);
        PaintContext layerContext {
            layerPainter,
            context.blitter,
            context.viewportSize
        };
        layerPainter.translate(-rect.topLeft());

        // Draw the children onto that layer.
        for (const auto& child : children)
        {
            child->execute(layerContext);
        }

        layerContext.painter.save();

        // Composite the finished layer onto the main surface.
        if (m_blendMode.has_value())
        {
            layerContext.painter.setCompositionMode(parseBlendMode(m_blendMode.value()));    
        }
        layerContext.painter.setOpacity(m_opacity);
        layerContext.painter.drawImage(0, 0, layer);
        layerContext.painter.restore();
    }    
    else 
    {
        for (const auto& child : children)
        {
            child->execute(context);
        }
    }
}

std::unique_ptr<Command> Blend::clone(std::unique_ptr<Command> child)
{
    std::vector<std::unique_ptr<Command>> children;
    children.push_back(std::move(child));
    return std::make_unique<Blend>(m_blendMode, m_opacity, std::move(children));
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
