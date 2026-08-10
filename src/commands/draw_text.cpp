#include "draw_text.h"

#include <QFontMetrics>
#include <QPainter>
#include <sstream>

/// Converts a CSS-style color: #RRGGBBAA to Qt-style: #AARRGGBB.
QColor colorFromCssHex(const QString& color)
{
    if (color.size() == 9 && color.startsWith('#')) 
    {
        return QColor("#" + color.right(2) + color.mid(1, 6));
    }

    return QColor(color);
}

DrawText::DrawText(int x1, int y1, QString text, QFont font, QString color)
    : m_text(text),
      m_font(font),
      m_color(color)
{
    QFontMetrics metrics(m_font);

    rect = { x1, y1, 
        x1 + metrics.horizontalAdvance(text), 
        metrics.ascent() + metrics.descent()
    };
}

void DrawText::execute(QImage* surface)
{
    QPainter painter(surface);
    auto color = colorFromCssHex(m_color);
    painter.setPen(color);
    painter.setFont(m_font);
    painter.drawText(rect, m_text);
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
