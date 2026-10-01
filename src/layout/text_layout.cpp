#include "text_layout.h"

#include "commands/draw_text.h"
#include "styles/style_helpers.h"
#include "helpers/string_helpers.h"
#include "html_tree/element.h"

#include <QFont>
#include <QFontMetrics>
#include <memory>
#include <sstream>

TextLayout::TextLayout(
    Node* node, 
    QString word,
    Layout* parent, 
    Layout* previous
)
{
    this->node = node;
    this->word = word;
    this->parent = parent;
    this->previous = previous;
    this->node->layoutObject = this;

    zoom = ProtectedField<double>(this, "zoom");
    font = ProtectedField<QFont>(this, "font");
    width = ProtectedField<double>(this, "width");
    height = ProtectedField<double>(this, "height");
    x = ProtectedField<double>(this, "x");
    y = ProtectedField<double>(this, "y");
    ascent = ProtectedField<int>(this, "ascent");
    descent = ProtectedField<int>(this, "descent");
 
    hasDirtyDescendants = true;
}

bool TextLayout::layoutNeeded() const
{
    if (zoom.isDirty()) return true;
    if (width.isDirty()) return true;
    if (hasDirtyDescendants) return true;
    return false;
}

void TextLayout::layout()
{
    if (!layoutNeeded()) return;

    zoom.copy(&parent->zoom);

    // Handle <big> and <small> tags by adjusting the font size, if found.
    int adjustSize = 0;
    if (const auto& element = dynamic_cast<const Element*>(node->parent))
    {
        if (element->tag == "small")
        {
            adjustSize = -2;
        }
        else if (element->tag == "big")
        {
            adjustSize = 4;
        }
    }

    auto zoom = this->zoom.read(&font);
    auto styles = node->styles.read(&font);
    font.set(getFont(parent->node, zoom, adjustSize, &font));

    {
        QFontMetrics metrics(font.read(&width));
        width.set(metrics.horizontalAdvance(word));
    }

    {
        QFontMetrics metrics(font.read(&ascent));
        ascent.set(metrics.ascent() * 1.25);
    }

    {
        QFontMetrics metrics(font.read(&descent));
        descent.set(metrics.descent() * 1.25);
    }

    {
        QFontMetrics metrics(font.read(&height));
        height.set(metrics.lineSpacing() * 1.25);
    }

    if (previous)
    {
        auto prevFont = previous->font.read(&x);
        QFontMetrics previousMetrics(prevFont);
        auto space = previousMetrics.horizontalAdvance(" ");
        auto prevX = previous->x.read(&x);
        auto prevWidth = previous->width.read(&x);
        x.set(prevX + space + prevWidth);
    }
    else 
    {
        x.copy(&parent->x);
    }

    hasDirtyDescendants = false;
}

bool TextLayout::shouldPaint() const 
{
    return true;
}

std::vector<std::unique_ptr<Command>> TextLayout::paint() const
{
    std::vector<std::unique_ptr<Command>> commands;

    auto colorStyle = node->findStyle("color", "black");
    auto color = QString::fromStdString(colorStyle);
    auto command = std::make_unique<DrawText>(x.get(), y.get(), word, font.get(), color);
    commands.push_back(std::move(command));

    return commands;
}

std::vector<std::unique_ptr<Command>> TextLayout::paintEffects(std::vector<std::unique_ptr<Command>> commands) const
{
    return commands;
}

std::string TextLayout::describe() const
{
    std::stringstream s; 
    s << "TextLayout(rect: {"
      << x.get() << ", " << y.get() << ", " << width.get() << ", " << height.get()
      << "}, node: " << trim(node->describe()) << ")";
    return s.str();
}
