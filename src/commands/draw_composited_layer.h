#pragma once

#include "composited_layer.h"
#include "paint_command.h"

class DrawCompositedLayer : public PaintCommand
{
public:
    DrawCompositedLayer(CompositedLayer* compositedLayer);
    void execute(PaintContext& context) override;
    std::string describe() const override;
    std::unique_ptr<Command> clone(std::unique_ptr<Command> child) override;

private:
    CompositedLayer* m_compositedLayer;
};
