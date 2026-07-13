#include "draw_rect.h"

DrawRect::DrawRect(int x1, int y1, int x2, int y2, QString color)
    : m_color(color)
{
    left = x1;
    top = y1;
    right = x2;
    bottom = y2;
}

void DrawRect::execute(QGraphicsScene* scene, int scroll)
{
    auto pen = QPen();
    auto brush = QBrush(QColor(m_color));
    int padding = 2;
    int x = left;
    int y = top - scroll + padding;
    int width = right;
    int height = bottom - top + padding;
    scene->addRect(x, y, width, height, pen, brush);
}

std::string DrawRect::describe() const
{
    std::stringstream s; 
    s << "DrawRect { ";
    s << "left: " << left << ", ";
    s << "top: " << top << ", ";
    s << "right: " << right << ", ";
    s << "bottom: " << bottom << ", ";
    s << "color: " << m_color.toStdString();
    s << " }";
    return s.str();
}
