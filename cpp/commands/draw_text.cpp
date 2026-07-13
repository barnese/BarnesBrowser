#include "draw_text.h"

#include <QFontMetrics>
#include <QGraphicsTextItem>

DrawText::DrawText(int x1, int y1, QString text, QFont font)
    : m_text(text),
      m_font(font)
{
    QFontMetrics metrics(m_font);
    left = x1;
    bottom = y1 + metrics.lineSpacing();
    top = y1;
}

void DrawText::execute(QGraphicsScene* scene, int scroll)
{
    auto text = scene->addText(m_text, m_font);
    text->setPos(left, top - scroll);
}
