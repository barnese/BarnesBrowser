#pragma once

#include "command.h"
#include "html_tree/node.h"

#include <QString>
#include <memory>
#include <vector>

/// Defines a command for opacity.
class Opacity : public Command
{
public:
    Opacity(double opacity, std::vector<std::unique_ptr<Command>> children);
    void execute(QImage* surface) override;
    std::string describe() const override;

private:
    double m_opacity;
    std::vector<std::unique_ptr<Command>> m_children;
};

std::vector<std::unique_ptr<Command>> paintVisualEffects(Node* node, std::vector<std::unique_ptr<Command>> commands, QRect);
