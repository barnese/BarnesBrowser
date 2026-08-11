#pragma once

#include "command.h"

#include <QString>

/// Defines a command for drawing a rounded rectangle.
class DrawRoundedRect : public Command
{
public:
    DrawRoundedRect(int x1, int y1, int x2, int y2, QString color, double radius);
    DrawRoundedRect(QRect rect, QString color, double radius);
    void execute(QImage* surface) override;
    std::string describe() const override;

private:
    QString m_color;
    double m_radius;
};
