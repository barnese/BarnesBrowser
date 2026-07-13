#pragma once

#include "command.h"

#include <QFont>
#include <QString>

/// Defines a command for drawing text.
class DrawText : public Command
{
public:
    DrawText(int x1, int y1, QString text, QFont font);
    void execute(QGraphicsScene* scene, int scroll) override;
    std::string describe() const override;

private:
    QString m_text;
    QFont m_font;
};
