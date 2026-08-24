#pragma once

#include "paint_command.h"

#include <QFont>
#include <QString>

/// Defines a command for drawing text.
class DrawText : public PaintCommand
{
public:
    DrawText(int x1, int y1, QString text, QFont font, QString color);
    void execute(PaintContext& context) override;
    std::string describe() const override;
    std::unique_ptr<Command> clone(std::unique_ptr<Command> child) override;

private:
    QString m_text;
    QFont m_font;
    QString m_color;
};
