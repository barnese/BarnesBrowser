#pragma once

#include "paint_command.h"

#include <QString>

/// Defines a command for drawing an image.
class DrawImage : public PaintCommand
{
public:
    DrawImage(QRect rect, QImage image, std::string quality);
    void execute(PaintContext& context) override;
    std::string describe() const override;

private:
    QImage m_image;
    std::string m_quality;
};
