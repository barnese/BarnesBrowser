#include "style_helpers.h"

#include "constants.h"
#include "helpers/font_cache.h"
#include "layout/layout_helpers.h"
#include "helpers/string_helpers.h"
#include "html_tree/element.h"

QFont::Weight cssFontWeightToQt(const std::string& input)
{
    if (input == "thin")       return QFont::Thin;
    if (input == "extralight") return QFont::ExtraLight;
    if (input == "light")      return QFont::Light;
    if (input == "normal")     return QFont::Normal;
    if (input == "medium")     return QFont::Medium;
    if (input == "semibold")   return QFont::DemiBold;
    if (input == "demibold")   return QFont::DemiBold;
    if (input == "bold")       return QFont::Bold;
    if (input == "extrabold")  return QFont::ExtraBold;
    if (input == "black")      return QFont::Black;

    // Fallback: try numeric.
    try 
    {
        int weight = std::stoi(input);
        return static_cast<QFont::Weight>(weight);
    } 
    catch (...) 
    {
        return QFont::Weight::Normal;
    }
}

QFont::Style cssFontStyleToQt(const std::string& style)
{
    if (style == "italic")  return QFont::Style::StyleItalic;
    if (style == "oblique") return QFont::Style::StyleOblique;
    return QFont::Style::StyleNormal;
}

void replaceHTMLCharacterEntities(QString& word)
{
    for (const auto& entity : Constants::HTML_CHARACTER_ENTITIES)
    {
        auto code = QString::fromStdString(entity.first);
        auto symbol = QString::fromStdString(entity.second);
        word.replace(code, symbol);
    }
}

double overrideSizeWithStyle(const Node* node, int size, std::string styleName)
{
    auto style = node->findStyle(styleName);
    return style.has_value() ? pixelsToDouble(style.value()) : size;
}

QFont getFont(const Node* node, double zoom, int adjustSize)
{    
    auto family = QString::fromStdString(node->findStyle("font-family", "Helvetica"));
    auto weight = cssFontWeightToQt(node->findStyle("font-weight", "normal"));
    auto style = cssFontStyleToQt(node->findStyle("font-style", "normal"));
    auto sizePx = pixelsToFloat(node->findStyle("font-size", "14px"));
    auto size = dpx(sizePx * 0.75 + adjustSize, zoom);

    return FontCache::instance().getFont(family, size, weight, style);
}
