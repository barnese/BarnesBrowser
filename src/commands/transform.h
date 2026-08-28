#pragma once

#include "visual_effect.h"

#include <optional>
#include <QPointF>
#include <QRect>

/// Helper function that maps a rect through a translation.
QRect mapTranslation(QRect rect, std::optional<QPointF> translation);

class Transform : public VisualEffect
{
public:
    Transform(
        std::optional<QPointF> translation, 
        QRect rect, 
        Node* node, 
        std::vector<std::unique_ptr<Command>> children
    );
    void execute(PaintContext& context) override;
    std::string describe() const override;
    std::unique_ptr<Command> clone(std::unique_ptr<Command> child) override;

private:
    std::optional<QPointF> m_translation;
};
