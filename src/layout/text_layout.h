#pragma once

#include "commands/command.h"
#include "html_tree/node.h"
#include "layout.h"

#include <memory>
#include <QString>
#include <vector>

class TextLayout : public Layout
{
public: 
    TextLayout(
        Node* node, 
        QString word, 
        const Layout* parent, 
        const Layout* previous = nullptr
    );
    void layout() override;
    std::vector<std::unique_ptr<Command>> paint() const override;
    bool shouldPaint() const override;
    std::vector<std::unique_ptr<Command>> paintEffects(std::vector<std::unique_ptr<Command>> commands) const override;
    std::string describe() const override;

private:
    QString word;
};
