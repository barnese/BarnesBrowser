#include "blend.h"

#include <QPainter>
#include <sstream>

Blend::Blend(std::string blendMode, std::vector<std::unique_ptr<Command>> children)
    : m_blendMode(blendMode),
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
    else 
    {
        return QPainter::CompositionMode_SourceOver;
    }
}

void Blend::execute(QImage* surface)
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
    painter.setCompositionMode(parseBlendMode(m_blendMode));    
    painter.drawImage(0, 0, layer);
}

std::string Blend::describe() const
{
    std::stringstream s; 
    s << "Opacity { ";
    s << "left: " << rect.left() << ", ";
    s << "top: " << rect.top() << ", ";
    s << "right: " << rect.right() << ", ";
    s << "bottom: " << rect.bottom() << ", ";
    s << "blend mode: " << m_blendMode;
    s << " }";
    return s.str();
}
