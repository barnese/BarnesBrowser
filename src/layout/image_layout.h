#pragma once

#include "commands/command.h"
#include "html_tree/node.h"
#include "layout.h"
#include "layout/embed_layout.h"
#include "frame.h"

#include <memory>
#include <vector>

/// Layout object representing an image element.
class ImageLayout : public EmbedLayout
{
public: 
    ImageLayout(
        Node* node, 
        Layout* parent, 
        Layout* previous,
        Frame* frame
    );
    void layout() override;
    std::vector<std::unique_ptr<Command>> paint() const override;
    std::string describe() const override;

private:
    double imageHeight;
};
