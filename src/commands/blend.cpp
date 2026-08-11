#include "blend.h"

#include <QPainter>
#include <sstream>

Blend::Blend(
    std::optional<std::string> blendMode, 
    double opacity,
    std::vector<std::unique_ptr<Command>> children
)
    : m_blendMode(blendMode),
      m_opacity(opacity),
      m_children(std::move(children))
{
    rect = {};

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

void Blend::execute(QImage* surface)
{
    // Only need to apply blending if a blend mode or opacity has been set.
    if (m_blendMode.has_value() || m_opacity < 1)
    {
        // Create an offscreen layer matching the destination.
        QImage layer(surface->size(), QImage::Format_RGBA8888);
        layer.fill(Qt::transparent);

        // Draw the children onto that layer.
        for (const auto& child : m_children)
        {
            child->execute(&layer);
        }

        // Composite the finished layer onto the main surface.
        QPainter painter(surface);
        if (m_blendMode.has_value())
        {
            painter.setCompositionMode(parseBlendMode(m_blendMode.value()));    
        }
        painter.setOpacity(m_opacity);
        painter.drawImage(0, 0, layer);
    }
    else 
    {
        for (const auto& child : m_children)
        {
            child->execute(surface);
        }
    }
}

std::string Blend::describe() const
{
    std::stringstream s; 
    s << "Opacity { ";
    s << "left: " << rect.left() << ", ";
    s << "top: " << rect.top() << ", ";
    s << "right: " << rect.right() << ", ";
    s << "bottom: " << rect.bottom() << ", ";
    s << "opacity: " << m_opacity << ", ";
    s << "blend mode: " << (m_blendMode.has_value() ? m_blendMode.value() : "not set");
    s << " }";
    return s.str();
}
