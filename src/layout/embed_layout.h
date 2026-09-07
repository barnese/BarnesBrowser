#pragma once

#include "html_tree/node.h"
#include "layout.h"

#include <QFont>
#include <QString>

/// Layout object representing an HTML embeddable element, 
/// such as an input or image.
class EmbedLayout : public Layout
{
public: 
    EmbedLayout(
        Node* node, 
        const Layout* parent, 
        const Layout* previous = nullptr
    );
    void layout() override;
    bool shouldPaint() const override;
};
