#pragma once

#include "paint_command.h"

#include <QImage>
#include <QRect>
#include <QString>

/// Defines a command for drawing an image.
class DrawImage : public PaintCommand
{
public:
    DrawImage(QImage* image, QRect rect, std::string quality);
    void execute(PaintContext& context) override;
    std::string describe() const override;

private:
    QImage* m_image;
    std::string m_quality;
};
