#include "draw_text.h"

#include <QFontMetrics>
#include <QGraphicsTextItem>

DrawText::DrawText(int x1, int y1, QString text, QFont font, QString color)
    : m_text(text),
      m_font(font),
      m_color(color)
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
    text->setDefaultTextColor(QColor(m_color));
}

std::string DrawText::describe() const
{
    std::stringstream s; 
    s << "DrawText { ";
    s << "left: " << left << ", ";
    s << "top: " << top << ", ";
    s << "right: " << right << ", ";
    s << "bottom: " << bottom << ", ";
    s << "text: \"" << m_text.toStdString() << "\", ";
    s << "font: " << m_font.family().toStdString() << ", ";
    s << "color: " << m_color.toStdString();
    s << " }";
    return s.str();
}
