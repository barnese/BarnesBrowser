#pragma once

#include "paint_command.h"
#include "layout/layout.h"

#include <QString>

/// Defines a command for drawing a line.
class DrawLine : public PaintCommand
{
public:
    DrawLine(double x1, double y1, double x2, double y2, QString color, int thickness);
    void execute(PaintContext& context) override;
    std::string describe() const override;

private:
    QString m_color;
    int m_thickness;
};

/// Helper that creates a command to draw a cursor.
std::unique_ptr<DrawLine> drawCursor(const Layout* layout, int offset);
