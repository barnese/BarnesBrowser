#pragma once

#include "selector.h"

#include <QColor>
#include <QPointF>
#include <QString>

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

using StringMap = std::map<std::string, std::string>;

struct NodeStyles
{
    std::string media;
    std::unique_ptr<Selector> node;
    StringMap styles;
};

using CSSRules = std::map<std::string, NodeStyles>;
using SortedCSSRules = std::vector<std::pair<std::string, NodeStyles>>;

SortedCSSRules sort(const CSSRules& rules);
QColor parseColor(const QString& color);

using StringTupleMap = std::map<std::string, std::tuple<std::string, std::string, std::string>>;
StringTupleMap diffStyles(StringMap& oldStyle, StringMap& newStyle);

std::optional<QPointF> parseTransform(const std::string& transformString);

/// Defines a parser for parsing CSS. This class uses parsing
/// functions to parse the property-value format of stylesheets.
class CSSParser
{
public:
    CSSParser(std::string text);
    StringMap body();
    CSSRules parse();
    std::unique_ptr<Selector> selector();

private:
    void whitespace();
    std::string word();
    void literal(char literal);
    std::tuple<std::string, std::string> pair(std::vector<char> until);
    std::optional<char> ignoreUntil(std::vector<char> chars);
    std::string untilChars(std::vector<char> chars);
    std::tuple<std::string, std::string> mediaQuery();
    bool parseMedia(std::string& media);

    // The text to be parsed.
    std::string m_text;
    
    // The parser's current position parsing the text.
    size_t m_i;

    bool m_darkMedia;
};
