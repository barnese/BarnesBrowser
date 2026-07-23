#pragma once

#include "layout.h"
#include "html_tree/node.h"

#include <QGraphicsScene>

/// Represents the layout of the root HTML document.
class DocumentLayout : public Layout
{
public:
    DocumentLayout(const Node* node, int parentWidth);
    void layout() override;
    std::vector<std::unique_ptr<Command>> paint() const override;

private:
    int m_parentWidth;
};
