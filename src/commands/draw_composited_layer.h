#pragma once

#include "composited_layer.h"
#include "paint_command.h"

class DrawCompositedLayer : public PaintCommand
{
public:
    DrawCompositedLayer(CompositedLayer* compositedLayer);
    void execute(PaintContext& context) override;
    std::string describe() const override;

private:
    CompositedLayer* m_compositedLayer;
};
