#include "draw_image.h"

#include <QPainter>
#include <sstream>

DrawImage::DrawImage(QImage* image, QRect r, std::string quality)
    : m_image(image),
      m_quality(quality)
{
    rect = r;
}

void DrawImage::execute(PaintContext& context)
{
    QPainter& painter = context.painter;

    // Store the painter's current state.
    painter.save();

    if (m_quality == "high-quality")
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    else
        painter.setRenderHint(QPainter::SmoothPixmapTransform, false);   
    
    painter.drawImage(rect, *m_image);

    // Restore the painter's original state.
    painter.restore();
}

std::string DrawImage::describe() const
{
    std::stringstream s; 
    s << "DrawImage(rect: {";
    s << rect.left() << ", " << rect.top() << ", " << rect.right() << ", " << rect.bottom() << "})";
    return s.str();
}
