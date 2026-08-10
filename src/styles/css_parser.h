#pragma once

#include "selector.h"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

struct NodeStyles
{
    std::unique_ptr<Selector> node;
    std::map<std::string, std::string> styles;
};

using CSSRules = std::map<std::string, NodeStyles>;
using SortedCSSRules = std::vector<std::pair<std::string, NodeStyles>>;

SortedCSSRules sort(const CSSRules& rules);

/// Defines a parser for parsing CSS. This class uses parsing
/// functions to parse the property-value format of stylesheets.
class CSSParser
{
public:
    CSSParser(std::string text);
    std::map<std::string, std::string> body();
    CSSRules parse();
    std::unique_ptr<Selector> selector();

private:
    void whitespace();
    std::string word();
    void literal(char literal);
    std::tuple<std::string, std::string> pair();
    std::optional<char> ignoreUntil(std::vector<char> chars);

    // The text to be parsed.
    std::string m_text;
    
    // The parser's current position parsing the text.
    size_t m_i;
};
