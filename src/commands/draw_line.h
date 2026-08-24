#pragma once

#include "paint_command.h"

#include <QString>

/// Defines a command for drawing a line.
class DrawLine : public PaintCommand
{
public:
    DrawLine(int x1, int y1, int x2, int y2, QString color, int thickness);
    void execute(PaintContext& context) override;
    std::string describe() const override;
    std::unique_ptr<Command> clone(std::unique_ptr<Command> child) override;

private:
    QString m_color;
    int m_thickness;
};
