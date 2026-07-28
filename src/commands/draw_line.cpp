#include "draw_line.h"

DrawLine::DrawLine(int x1, int y1, int x2, int y2, QString color, int thickness)
    : m_color(color),
      m_thickness(thickness)
{
    left = x1;
    top = y1;
    right = x2;
    bottom = y2;
}

void DrawLine::execute(QGraphicsScene* scene, int scroll)
{
    int x1 = left;
    int y1 = top - scroll;
    int x2 = right;
    int y2 = bottom - scroll;
    auto pen = QPen(QColor(m_color), m_thickness);
    scene->addLine(x1, y1, x2, y2, pen);
}

std::string DrawLine::describe() const
{
    std::stringstream s; 
    s << "DrawLine { ";
    s << "left: " << left << ", ";
    s << "top: " << top << ", ";
    s << "right: " << right << ", ";
    s << "bottom: " << bottom << ", ";
    s << "color: " << m_color.toStdString() << ", ";
    s << "thickness: " << m_thickness;
    s << " }";
    return s.str();
}
