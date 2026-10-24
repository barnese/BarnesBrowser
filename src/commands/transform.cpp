#include "transform.h"

#include <sstream>

Transform::Transform(
    std::optional<QPointF> translation, 
    QRectF rect, 
    Node* node, 
    std::vector<std::unique_ptr<Command>> children
) : VisualEffect(rect, std::move(children), node), m_translation(translation)
{
}

void Transform::execute(PaintContext& context)
{
    if (m_translation.has_value())
    {
        context.painter.save();
        context.painter.translate(m_translation.value());
    }
    
    for (const auto& command : children)
    {
        command->execute(context);
    }

    if (m_translation.has_value())
    {
        context.painter.restore();
    }
}

std::unique_ptr<Command> Transform::clone(std::unique_ptr<Command> child)
{
    return std::make_unique<Transform>(m_translation, rect, node, commandList(std::move(child)));
}

std::string Transform::describe() const
{
    std::stringstream s; 
    s << "Transform(translate(";
    if (m_translation.has_value())
    {
        s << m_translation.value().x()
          << ", " 
          << m_translation.value().y();
    }
    else
    {   
        s << "no-op";
    }
    s << "))";
    return s.str();
}

QRectF mapTranslation(QRectF rect, std::optional<QPointF> translation, bool reversed)
{
    if (!translation.has_value())
        return rect;

    QTransform matrix;
    if (reversed)
        matrix.translate(-translation->x(), -translation->y());
    else
        matrix.translate(translation->x(), translation->y());
    return matrix.mapRect(rect);
}

QRectF Transform::map(QRectF rect) const
{
    return mapTranslation(rect, m_translation, false);
}

QRectF Transform::unmap(QRectF rect) const
{
    return mapTranslation(rect, m_translation, true);
}
