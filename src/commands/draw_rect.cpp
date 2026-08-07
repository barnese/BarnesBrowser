#include "draw_rect.h"

#include <QPainter>
#include <sstream>

DrawRect::DrawRect(int x1, int y1, int x2, int y2, QString color)
    : m_color(color)
{
    rect = {x1, y1, x2, y2};
}

void DrawRect::execute(QImage* surface, int scroll)
{
    QRect scrolledRect = {
        rect.left(),
        rect.top() - scroll,
        rect.width(),
        rect.height()
    };

    QPainter painter(surface);
    auto brush = QBrush(QColor(m_color));
    painter.setBrush(brush);
    painter.drawRect(scrolledRect);
}

std::string DrawRect::describe() const
{
    std::stringstream s; 
    s << "DrawRect { ";
    s << "left: " << rect.left() << ", ";
    s << "top: " << rect.top() << ", ";
    s << "right: " << rect.right() << ", ";
    s << "bottom: " << rect.bottom() << ", ";
    s << "color: " << m_color.toStdString();
    s << " }";
    return s.str();
}
