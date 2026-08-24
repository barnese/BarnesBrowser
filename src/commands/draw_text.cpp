#include "draw_text.h"

#include "styles/css_parser.h"

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
        metrics.ascent() + metrics.descent()
    };
}

void DrawText::execute(PaintContext& context)
{
    QPainter& painter = context.painter;
    auto color = parseColor(m_color);
    painter.setPen(color);
    painter.setFont(m_font);
    painter.drawText(rect, m_text);
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

std::unique_ptr<Command> DrawText::clone(std::unique_ptr<Command>)
{
    return nullptr;
}
