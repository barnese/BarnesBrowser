#include "draw_text.h"

#include "styles/css_parser.h"

#include <QFontMetrics>
#include <QPainter>
#include <sstream>

DrawText::DrawText(double x1, double y1, QString text, QFont font, QString color)
    : m_text(text),
      m_font(font),
      m_color(color)
{
    QFontMetrics metrics(m_font);
    double ascent = metrics.ascent();
    double descent = metrics.descent();

    // Slightly adjust y and height to account for emoji fonts.
    rect = { 
        x1, 
        y1 - 2, 
        x1 + metrics.horizontalAdvance(text), 
        ascent + descent + 2
    };
}

void DrawText::execute(PaintContext& context)
{
    QPainter& painter = context.painter;
    auto color = parseColor(m_color);
    painter.setPen(color);
    painter.setFont(m_font);
    
    // Align text left/centered to fix emojis.
    painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, m_text);
}

std::string DrawText::describe() const
{
    std::stringstream s; 
    s << "DrawText(rect: {";
    s << rect.left() << ", " << rect.top() << ", " << rect.right() << ", " << rect.bottom() << "}, ";
    s << "text: \"" << m_text.toStdString() << "\", ";
    s << "color: " << m_color.toStdString();
    s << ")";
    return s.str();
}
