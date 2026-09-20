#pragma once

#include "commands/command.h"
#include "html_tree/node.h"
#include "layout.h"
#include "layout/embed_layout.h"
#include "frame.h"

#include <memory>
#include <vector>

/// Layout object representing an HTML input area.
class InputLayout : public EmbedLayout
{
public: 
    InputLayout(
        Node* node, 
        Layout* parent, 
        Layout* previous,
        Frame* frame
    );
    void layout() override;
    std::vector<std::unique_ptr<Command>> paint() const override;
    bool shouldPaint() const override;
    std::vector<std::unique_ptr<Command>> paintEffects(std::vector<std::unique_ptr<Command>> commands) const override;
    std::string describe() const override;
};
