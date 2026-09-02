#include "draw_line.h"

#include "styles/css_parser.h"

#include <QPainter>
#include <sstream>

DrawLine::DrawLine(int x1, int y1, int x2, int y2, QString color, int thickness)
    : m_color(color),
      m_thickness(thickness)
{
    rect = {x1, y1, x2 - x1, y2 - y1};
}

void DrawLine::execute(PaintContext& context)
{
    QPainter& painter = context.painter;
    QLine line(
        rect.left(),
        rect.top(),
        rect.right(),
        rect.bottom()
    );
    
    auto color = parseColor(m_color);
    auto pen = QPen(color, m_thickness);
    painter.setPen(pen);
    painter.drawLine(line);
}

std::string DrawLine::describe() const
{
    std::stringstream s; 
    s << "DrawLine(rect: {";
    s << rect.left() << ", " << rect.top() << ", " << rect.right() << ", " << rect.bottom() << "}, ";
    s << "color: " << m_color.toStdString() << ", ";
    s << "thickness: " << m_thickness;
    s << ")";
    return s.str();
}
