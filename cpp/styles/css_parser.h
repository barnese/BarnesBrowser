#pragma once

#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

/// Defines a parser for parsing CSS. This class uses parsing
/// functions to parse the property-value format of stylesheets.
class CSSParser
{
public:
    CSSParser(std::string text);
    std::map<std::string, std::string> body();

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
