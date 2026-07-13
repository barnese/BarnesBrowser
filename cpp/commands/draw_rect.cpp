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
    scene->addRect(left, top - scroll, right, bottom, QPen(), QBrush(QColor(m_color)));
}
