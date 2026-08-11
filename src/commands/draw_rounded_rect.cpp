#include "draw_rounded_rect.h"

#include "styles/css_parser.h"

#include <QPainter>
#include <QPen>
#include <sstream>

DrawRoundedRect::DrawRoundedRect(int x1, int y1, int x2, int y2, QString color, double radius)
    : m_color(color),
      m_radius(radius)
{
    rect = {x1, y1, x2, y2};
}

DrawRoundedRect::DrawRoundedRect(QRect r, QString color, double radius)
    : m_color(color),
      m_radius(radius)
{
    rect = r;
}

void DrawRoundedRect::execute(QImage* surface)
{
    QPainter painter(surface);

    auto color = parseColor(m_color);
    auto brush = QBrush(color);
    painter.setBrush(brush);
    auto pen = QPen(Qt::transparent);
    painter.setPen(pen);
    painter.drawRoundedRect(rect, m_radius, m_radius);
}

std::string DrawRoundedRect::describe() const
{
    std::stringstream s; 
    s << "DrawRoundedRect { ";
    s << "left: " << rect.left() << ", ";
    s << "top: " << rect.top() << ", ";
    s << "right: " << rect.right() << ", ";
    s << "bottom: " << rect.bottom() << ", ";
    s << "color: " << m_color.toStdString() << ", ";
    s << "radius: " << m_radius;
    s << " }";
    return s.str();
}
