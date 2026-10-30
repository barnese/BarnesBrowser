#pragma once

#include "commands/command.h"
#include "html_tree/node.h"
#include "layout.h"
#include "text_alignment.h"

#include <memory>
#include <vector>

class LineLayout : public Layout
{
public: 
    LineLayout(
        Node* node, 
        Layout* parent, 
        Layout* previous = nullptr,
        TextAlignment alignment = LEFT
    );
    bool layoutNeeded() const override;
    void layout() override;
    std::vector<std::unique_ptr<Command>> paint() const override;
    bool shouldPaint() const override;
    std::vector<std::unique_ptr<Command>> paintEffects(std::vector<std::unique_ptr<Command>> commands) const override;
    std::string describe() const override;

private:
    void layoutAlignment();
    
    bool initializedFields;
    TextAlignment m_alignment;
};
