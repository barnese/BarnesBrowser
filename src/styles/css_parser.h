#pragma once

#include "selector.h"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

using CSSRules = std::map<std::unique_ptr<Selector>, std::map<std::string, std::string>>;
using SortedCSSRules = std::vector<std::pair<std::unique_ptr<Selector>, std::map<std::string, std::string>>>;

SortedCSSRules sort(const CSSRules& rules);

/// Defines a parser for parsing CSS. This class uses parsing
/// functions to parse the property-value format of stylesheets.
class CSSParser
{
public:
    CSSParser(std::string text);
    std::map<std::string, std::string> body();
    CSSRules parse();

private:
    void whitespace();
    std::string word();
    void literal(char literal);
    std::tuple<std::string, std::string> pair();
    std::optional<char> ignoreUntil(std::vector<char> chars);
    std::unique_ptr<Selector> selector();

    // The text to be parsed.
    std::string m_text;
    
    // The parser's current position parsing the text.
    size_t m_i;
};
