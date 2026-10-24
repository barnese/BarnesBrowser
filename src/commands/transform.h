#pragma once

#include "visual_effect.h"

#include <optional>
#include <QPointF>
#include <QRect>

/// Helper function that maps a rect through a translation.
QRectF mapTranslation(QRectF rect, std::optional<QPointF> translation, bool reversed);

class Transform : public VisualEffect
{
public:
    Transform(
        std::optional<QPointF> translation, 
        QRectF rect, 
        Node* node, 
        std::vector<std::unique_ptr<Command>> children
    );
    void execute(PaintContext& context) override;
    std::string describe() const override;
    std::unique_ptr<Command> clone(std::unique_ptr<Command> child) override;
    QRectF map(QRectF rect) const override;
    QRectF unmap(QRectF rect) const override;

private:
    std::optional<QPointF> m_translation;
};
