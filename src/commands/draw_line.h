#pragma once

#include "command.h"

#include <QString>

/// Defines a command for drawing a line.
class DrawLine : public Command
{
public:
    DrawLine(int x1, int y1, int x2, int y2, QString color, int thickness);
    void execute(QImage* surface, int scroll) override;
    std::string describe() const override;

private:
    QString m_color;
    int m_thickness;
};
