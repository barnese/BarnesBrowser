#include "draw_outline.h"

#include "styles/css_parser.h"

#include <QPainter>
#include <QPen>
#include <sstream>

DrawOutline::DrawOutline(QRectF r, std::string color, int thickness)
    : m_color(QString::fromStdString(color)),
      m_thickness(thickness)
{
    rect = r;
}

void DrawOutline::execute(PaintContext& context)
{
    QPainter& painter = context.painter;
    auto color = parseColor(m_color);
    auto pen = QPen(color);
    pen.setWidth(m_thickness);
    
    painter.setBrush(Qt::NoBrush);
    painter.setPen(pen);
    painter.drawRect(rect);
}

std::string DrawOutline::describe() const
{
    std::stringstream s; 
    s << "DrawOutline(rect: {";
    s << rect.left() << ", " << rect.top() << ", " << rect.right() << ", " << rect.bottom() << "}, ";
    s << "color: " << m_color.toStdString() << ", ";
    s << "thickness: " << m_thickness;
    s << ")";
    return s.str();
}
