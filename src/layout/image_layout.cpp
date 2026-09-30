#include "image_layout.h"

#include "commands/draw_image.h"
#include "html_tree/element.h"
#include "layout_helpers.h"

#include <QFontMetricsF>
#include <memory>
#include <iostream>
#include <sstream>

ImageLayout::ImageLayout(
    Node* node, 
    Layout* parent, 
    Layout* previous,
    Frame* frame
) : EmbedLayout(node, parent, previous, frame)
{
}

void ImageLayout::layout()
{
    if (!layoutNeeded()) return;
    EmbedLayout::layout();

    // TODO: replace this with a read(&width) and read(&height)
    auto zoom = this->zoom.get();

    if (auto const& element = dynamic_cast<Element*>(node))
    {   
        auto widthAttr = element->findAttribute("width");
        auto heightAttr = element->findAttribute("height");
        double imgWidth = element->image->width();
        double imgHeight = element->image->height();
        double aspectRatio = imgWidth / imgHeight;

        if (widthAttr.has_value() && heightAttr.has_value())
        {
            width.set(dpx(std::stod(widthAttr.value()), zoom));
            imageHeight = dpx(std::stod(heightAttr.value()), zoom);
        }
        else if (widthAttr.has_value())
        {
            width.set(dpx(std::stod(widthAttr.value()), zoom));
            imageHeight = width.read(&height) / aspectRatio;
        }
        else if (heightAttr.has_value())
        {
            imageHeight = dpx(std::stod(heightAttr.value()), zoom);
            width.set(imageHeight * aspectRatio);
        }
        else 
        {    
            width.set(dpx(imgWidth, zoom));
            imageHeight = dpx(imgHeight, zoom);
        }

        QFontMetricsF metrics(font.read(&height));
        height.set(std::max(imageHeight, metrics.lineSpacing()));

        ascent.set(height.read(&ascent));
        descent.set(0);
    }
}

std::vector<std::unique_ptr<Command>> ImageLayout::paint() const
{
    std::vector<std::unique_ptr<Command>> commands;

    if (auto const& element = dynamic_cast<Element*>(node))
    {
        QRect rect(x.get(), y.get() + height.get() - imageHeight, width.get(), height.get());
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
      << x.get() << ", " << y.get() << ", " << width.get() << ", " << height.get()
      << "}, node: " << node->describe() << ")";
    return s.str();
}
