#pragma once

#include "html_tree/node.h"
#include "layout.h"
#include "frame.h"

#include <QFont>
#include <QRect>
#include <QString>

/// Layout object representing an HTML embeddable element, 
/// such as an input or image.
class EmbedLayout : public Layout
{
public: 
    EmbedLayout(
        Node* node, 
        const Layout* parent, 
        const Layout* previous,
        Frame* frame
    );
    void layout() override;
    std::vector<std::unique_ptr<Command>> paint() const override;
    bool shouldPaint() const override;
    std::vector<std::unique_ptr<Command>> paintEffects(std::vector<std::unique_ptr<Command>> commands) const override;
    std::string describe() const override;
};
