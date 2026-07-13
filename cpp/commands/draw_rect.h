#pragma once

#include "command.h"

#include <QString>

/// Defines a command for drawing a rectangle.
class DrawRect : public Command
{
public:
    DrawRect(int x1, int y1, int x2, int y2, QString color);
    void execute(QGraphicsScene* scene, int scroll) override;
    std::string describe() const override;

private:
    QString m_color;
};
