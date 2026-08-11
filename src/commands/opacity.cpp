#include "opacity.h"

#include <QPainter>
#include <sstream>

Opacity::Opacity(double opacity, std::vector<std::unique_ptr<Command>> children)
    : m_opacity(opacity),
      m_children(std::move(children))
{
    rect = {};

    // Expand the bounding box to enclose each child's rectangle.
    for (const auto& child : children)
    {
        rect = rect.united(child->rect);
    }
}

void Opacity::execute(QImage* surface)
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
    painter.setOpacity(m_opacity);
    painter.drawImage(0, 0, layer);
}

std::string Opacity::describe() const
{
    std::stringstream s; 
    s << "Opacity { ";
    s << "left: " << rect.left() << ", ";
    s << "top: " << rect.top() << ", ";
    s << "right: " << rect.right() << ", ";
    s << "bottom: " << rect.bottom() << ", ";
    s << "opacity: " << m_opacity;
    s << " }";
    return s.str();
}
