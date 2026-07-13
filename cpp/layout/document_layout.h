#pragma once

#include "layout.h"
#include "html_tree/node.h"

#include <QGraphicsScene>

/// Represents the layout of the root HTML document.
class DocumentLayout : public Layout
{
public:
    DocumentLayout(const Node* node);
    void layout() override;
    DisplayList paint() const override;
};
