#include "css_parser.h"

#include "css_parser_exception.h"
#include "helpers/string_helpers.h"

#include <algorithm>

CSSParser::CSSParser(std::string text)
    : m_text(text),
      m_i(0)
{
}

void CSSParser::whitespace()
{
    while (m_i < m_text.size() && std::isspace(m_text[m_i]))
    {
        m_i += 1;
    }
}

std::string CSSParser::word()
{
    size_t start = m_i;
    while (m_i < m_text.size())
    {
        std::vector<char> specials = {'#', '-', '.', '%'};
        if (std::isalnum(m_text[m_i]) || std::ranges::contains(specials, m_text[m_i]))
        {
            m_i += 1;
        }
        else 
        {
            break;
        }
    }
    
    if (!(m_i > start))
    {
        throw CSSParserException("word parsing error");
    }

    return m_text.substr(start, m_i - start);
}

void CSSParser::literal(char literal)
{
    if (!(m_i < m_text.size()) && m_text[m_i] == literal)
    {
        throw CSSParserException("literal parsing error");
    }
    m_i += 1;
}

std::tuple<std::string, std::string> CSSParser::pair()
{
    auto property = word();
    whitespace();
    literal(':');
    whitespace();
    auto value = word();
    return { toLowercase(property), value };
}

std::map<std::string, std::string> CSSParser::body()
{
    std::map<std::string, std::string> pairs;

    // Parse sequences of styles by calling parsing functions in a loop.
    while (m_i < m_text.size())
    {
        try
        {
            auto [property, value] = pair();
            pairs[property] = value;
            whitespace();
            literal(';');
            whitespace();
        }
        catch (const CSSParserException& e)
        {
            // We failed to parse a property-value pair, so skip either 
            // to the next semicolon or to the end of the string.
            auto why = ignoreUntil({';'});
            if (why == ';')
            {
                literal(';');
                whitespace();
            }
            else 
            {
                break;
            }
        }
    }

    return pairs;
}

std::optional<char> CSSParser::ignoreUntil(std::vector<char> chars)
{
    while (m_i < m_text.size())
    {
        if (std::ranges::contains(chars, m_text[m_i]))
        {
            return m_text[m_i];
        }
        else 
        {
            m_i += 1;
        }
    }

    return {};
}
