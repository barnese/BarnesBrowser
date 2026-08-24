#pragma once

#include "paint_command.h"

#include <QString>

/// Defines a command for drawing a rounded rectangle.
class DrawRoundedRect : public PaintCommand
{
public:
    DrawRoundedRect(int x1, int y1, int x2, int y2, QString color, double radius);
    DrawRoundedRect(QRect rect, QString color, double radius);
    void execute(PaintContext& context) override;
    std::string describe() const override;
    std::unique_ptr<Command> clone(std::unique_ptr<Command> child) override;

private:
    QString m_color;
    double m_radius;
};
