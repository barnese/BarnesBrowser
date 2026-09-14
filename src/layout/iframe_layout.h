#pragma once

#include "commands/command.h"
#include "embed_layout.h"
#include "html_tree/node.h"
#include "layout/layout.h"
#include "frame.h"

#include <memory>
#include <QRect>
#include <string>
#include <vector>

class IFrameLayout : public EmbedLayout
{
public:
    IFrameLayout(
        Node* node, 
        const Layout* parent, 
        const Layout* previous,
        Frame* parentFrame
    );
    void layout() override;
    std::string describe() const override;
    std::vector<std::unique_ptr<Command>> paint() const override;
    std::vector<std::unique_ptr<Command>> paintEffects(std::vector<std::unique_ptr<Command>> commands) const override;
};
