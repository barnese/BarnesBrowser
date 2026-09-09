#include "image_layout.h"

#include "commands/draw_image.h"
#include "styles/style_helpers.h"
#include "html_tree/element.h"
#include "layout_helpers.h"

#include <QFontMetrics>
#include <QRect>
#include <memory>
#include <iostream>
#include <sstream>

ImageLayout::ImageLayout(
    Node* node, 
    const Layout* parent, 
    const Layout* previous
) : EmbedLayout(node, parent, previous)
{
}

void ImageLayout::layout()
{
    EmbedLayout::layout();

    if (auto const& element = dynamic_cast<Element*>(node))
    {        
        width = dpx(element->image->width(), zoom);
        imageHeight = dpx(element->image->height(), zoom);
        font = getFont(node, zoom);
        QFontMetrics metrics(font);
        height = std::max(imageHeight, metrics.lineSpacing());
        ascent = height;
        descent = 0;
    }
}

std::vector<std::unique_ptr<Command>> ImageLayout::paint() const
{
    std::vector<std::unique_ptr<Command>> commands;

    if (auto const& element = dynamic_cast<Element*>(node))
    {
        QRect rect(x, y + height - imageHeight, x + width, y + height);
        auto quality = node->findStyle("image-rendering", "auto");
        commands.push_back(
            std::make_unique<DrawImage>(element->image.get(), rect, quality)
        );
    }

    return commands;
}

std::string ImageLayout::describe() const
{
    std::stringstream s; 
    s << "ImageLayout(rect: {"
      << x << ", " << y << ", " << width << ", " << height
      << "}, node: " << node->describe() << ")";
    return s.str();
}
