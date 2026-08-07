#include "draw_text.h"

#include <QFontMetrics>
#include <QPainter>
#include <sstream>

DrawText::DrawText(int x1, int y1, QString text, QFont font, QString color)
    : m_text(text),
      m_font(font),
      m_color(color)
{
    QFontMetrics metrics(m_font);

    rect = { x1, y1, 
        x1 + metrics.horizontalAdvance(text), 
        y1 - metrics.ascent() + metrics.descent()
    };
}

void DrawText::execute(QImage* surface, int scroll)
{
    QRect scrolledRect = {
        rect.left(),
        rect.top() - scroll,
        rect.right(),
        rect.bottom() - scroll
    };
    
    QPainter painter(surface);
    painter.setPen(m_color);
    painter.setFont(m_font);
    painter.drawText(scrolledRect, m_text);
}

std::string DrawText::describe() const
{
    std::stringstream s; 
    s << "DrawText { ";
    s << "left: " << rect.left() << ", ";
    s << "top: " << rect.top() << ", ";
    s << "right: " << rect.right() << ", ";
    s << "bottom: " << rect.bottom() << ", ";
    s << "text: \"" << m_text.toStdString() << "\", ";
    s << "font: " << m_font.family().toStdString() << ", ";
    s << "color: " << m_color.toStdString();
    s << " }";
    return s.str();
}
