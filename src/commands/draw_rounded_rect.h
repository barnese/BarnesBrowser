#pragma once

#include "paint_command.h"

#include <QRectF>
#include <QString>

/// Defines a command for drawing a rounded rectangle.
class DrawRoundedRect : public PaintCommand
{
public:
    DrawRoundedRect(QRectF rect, QString color, double radius);
    void execute(PaintContext& context) override;
    std::string describe() const override;

private:
    QString m_color;
    double m_radius;
};
