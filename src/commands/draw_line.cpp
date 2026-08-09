#include "draw_line.h"

#include <QPainter>
#include <sstream>

DrawLine::DrawLine(int x1, int y1, int x2, int y2, QString color, int thickness)
    : m_color(color),
      m_thickness(thickness)
{
    rect = {x1, y1, x2 - x1, y2 - y1};
}

void DrawLine::execute(QImage* surface)
{
    QLine line(
        rect.left(),
        rect.top(),
        rect.right(),
        rect.bottom()
    );
    
    QPainter painter(surface);
    auto pen = QPen(QColor(m_color), m_thickness);
    painter.setPen(pen);
    painter.drawLine(line);
}

std::string DrawLine::describe() const
{
    std::stringstream s; 
    s << "DrawLine { ";
    s << "left: " << rect.left() << ", ";
    s << "top: " << rect.top() << ", ";
    s << "right: " << rect.right() << ", ";
    s << "bottom: " << rect.bottom() << ", ";
    s << "color: " << m_color.toStdString() << ", ";
    s << "thickness: " << m_thickness;
    s << " }";
    return s.str();
}
