#include "embed_layout.h"

EmbedLayout::EmbedLayout(
    Node* node, 
    const Layout* parent, 
    const Layout* previous
)
{
    this->node = node;
    this->parent = parent;
    this->previous = previous;
    this->node->layoutObject = this;
}

void EmbedLayout::layout()
{
    zoom = parent->zoom;
    // TODO
}

bool EmbedLayout::shouldPaint() const
{
    return true;
}
