#pragma once

#include "paint_command.h"

#include <QString>

/// Defines a command for drawing an outlined rectangle.
class DrawOutline : public PaintCommand
{
public:
    DrawOutline(QRect rect, QString color, double zoom);
    void execute(PaintContext& context) override;
    std::string describe() const override;

private:
    QString m_color;
    double m_zoom;
};
