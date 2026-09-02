#include "draw_outline.h"

#include "styles/css_parser.h"

#include <QPainter>
#include <QPen>
#include <sstream>

DrawOutline::DrawOutline(QRect r, QString color, double zoom)
    : m_color(color),
      m_zoom(zoom)
{
    rect = r;
}

void DrawOutline::execute(PaintContext& context)
{
    QPainter& painter = context.painter;
    auto color = parseColor(m_color);
    auto pen = QPen(color);
    pen.setWidth(2);
    
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
    s << "zoom: " << m_zoom;
    s << ")";
    return s.str();
}
