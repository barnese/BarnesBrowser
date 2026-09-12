#include "css_parser.h"

#include "css_parser_exception.h"
#include "helpers/string_helpers.h"
#include "styles/descendant_selector.h"
#include "styles/pseudo_class_selector.h"
#include "styles/selector.h"
#include "styles/tag_selector.h"
#include "constants.h"

#include <algorithm>
#include <iostream>
#include <sstream>

CSSParser::CSSParser(std::string text)
    : m_text(text),
      m_i(0),
      m_darkMedia(false)
{
    // Remove comments from the entire text.
    m_text = removeComments(m_text);
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
    std::vector<char> specials = {',', '/', '#', '-', '.', '%', '(', ')', '"', '\''};
    size_t start = m_i;
    bool inQuote = false;
    while (m_i < m_text.size())
    {
        auto cur = m_text[m_i];

        if (cur == '\'')
        {
            inQuote = !inQuote;
        }
        
        if (std::isalnum(cur) || 
            std::ranges::contains(specials, cur) ||
            (inQuote && cur == ':'))
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

std::tuple<std::string, std::string> CSSParser::pair(std::vector<char> until)
{
    auto property = word();
    whitespace();
    literal(':');
    whitespace();
    auto value = untilChars(until);
    return { toLowercase(property), trim(value) };
}

StringMap CSSParser::body()
{
    StringMap pairs;

    // Parse sequences of styles by calling parsing functions in a loop.
    while (m_i < m_text.size() && m_text[m_i] != '}')
    {
        try
        {
            auto [property, value] = pair({';', '}'});
            pairs[property] = value;
            whitespace();
            literal(';');
            whitespace();
        }
        catch (const CSSParserException& e)
        {
            //std::cout << "Body failure: " << e.what() << std::endl;

            // We failed to parse a property-value pair, so skip either 
            // to the next semicolon or to the end of the string.
            auto why = ignoreUntil({';', '}'});
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

std::string CSSParser::untilChars(std::vector<char> chars)
{
    auto start = m_i;
    while (m_i < m_text.size() && !std::ranges::contains(chars, m_text[m_i]))
    {
        m_i += 1;
    }

    return m_text.substr(start, m_i - start);
}

std::tuple<std::string, std::string> CSSParser::mediaQuery()
{
    literal('@');
    if (word() != "media")
        throw CSSParserException("unhandled CSS @ query");
    whitespace();
    literal('(');
    whitespace();
    auto propAndVal = pair({')'});
    whitespace();
    literal(')');
    return propAndVal;
}

std::unique_ptr<Selector> CSSParser::simpleSelector()
{
    auto w = toLowercase(word());
    if (m_darkMedia)
        w = Constants::MEDIA_DARK_PREFIX + w;
    std::unique_ptr<Selector> out = std::make_unique<TagSelector>(w);

    if (m_i < m_text.size() && m_text[m_i] == ':')
    {
        literal(':');
        auto pseudoClass = toLowercase(word());
        out = std::make_unique<PseudoClassSelector>(pseudoClass, std::move(out));
    }

    return out;
}

std::unique_ptr<Selector> CSSParser::selector()
{
    auto out = simpleSelector();
    whitespace();
    while (m_i < m_text.size() && m_text[m_i] != '{')
    {
        auto descendant = simpleSelector();
        out = std::make_unique<DescendantSelector>(std::move(out), std::move(descendant));
        whitespace();
    }
    return out;
}

bool CSSParser::parseMedia(std::string& media)
{
    std::vector<std::string> supportedMediaTypes = { "dark", "light" };
    if (m_text[m_i] == '@' && media.empty())
    {
        auto [prop, val] = mediaQuery();
        if (prop == "prefers-color-scheme" && std::ranges::contains(supportedMediaTypes, val))
        {
            media = val;
            if (val == "dark")
                m_darkMedia = true;
        }
        whitespace();
        literal('{');
        whitespace();
        return true;
    }
    else if (m_text[m_i] == '}' && !media.empty())
    {
        literal('}');
        media = "";
        whitespace();
        m_darkMedia = false;
        return true;
    }
    return false;
}

CSSRules CSSParser::parse()
{
    CSSRules rules;
    std::string media;
    whitespace();
    while (m_i < m_text.size())
    {
        try 
        {
            if (parseMedia(media)) continue;
            auto selector = this->selector();
            literal('{');
            whitespace();
            auto body = this->body();
            literal('}');
            whitespace();
            auto name = selector->getName();
            rules[name] = NodeStyles(media, std::move(selector), body);
        } 
        catch (const CSSParserException& e) 
        {
            // We failed to parse the selector, so skip the whole rule.
            // std::cout << "Parse failure: " << e.what() << std::endl;
            auto why = ignoreUntil({'}'});
            if (why == '}')
            {
                literal('}');
                whitespace();
            }
            else 
            {
                break;
            }
        }
    }

    return rules;
}

QColor parseColor(const QString& color)
{
    // Qt orders alpha at the beginning of the string instead
    // of at the end like in CSS, so we need to swap the order.
    if (color.startsWith("#") && color.size() == 9)
    {
        return QColor("#" + color.right(2) + color.mid(1, 6));
    }

    return QColor(color);
}

StringMap parseTransition(std::string& value)
{
    StringMap properties;

    if (value.empty()) return properties;

    for (auto item : split(value, ",", false))
    {
        auto innerItems = split(item, " ");
        auto property = innerItems[0];
        auto duration = innerItems[1];
        // Remove the last character from the duration.
        double value = std::stod(duration.substr(0, duration.size() - 1));
        auto frames = (int)(value / (Constants::REFRESH_RATE_MS * 0.001));
        properties[property] = std::to_string(frames);
    }

    return properties;
}

/// @returns a map of only the transitioning properties, mapping
/// each property to its old value, new value an duration (in frames).
StringTupleMap diffStyles(StringMap& oldStyle, StringMap& newStyle)
{
    StringTupleMap transitions;

    auto it = newStyle.find("transition");
    if (it == newStyle.end()) return transitions;
    auto items = parseTransition(it->second);

    for (const auto& [property, numFrames] : items)
    {
        auto oldIt = oldStyle.find(property);
        if (oldIt == oldStyle.end()) continue;

        auto newIt = newStyle.find(property);
        if (newIt == newStyle.end()) continue;

        auto oldValue = oldIt->second;
        auto newValue = newIt->second;

        if (oldValue == newValue) continue;

        transitions[property] = std::make_tuple(oldValue, newValue, numFrames);
    }
    
    return transitions;
}

std::optional<QPointF> parseTransform(const std::string& transformString)
{
    if (!transformString.contains("translate("))
        return std::nullopt;

    auto leftParen = transformString.find('(');
    auto rightParen = transformString.find(')');
    auto innerText = transformString.substr(leftParen + 1, rightParen - leftParen);
    auto values = split(innerText, ",");
    auto x = pixelsToFloat(values[0]);
    auto y = pixelsToFloat(values[1]);
    return QPointF(x, y);
}

void printStyles(const CSSRules& rules)
{
    std::stringstream s;
    s << "Styles:" << std::endl;

    for (const auto& rule : rules)
    {
        s << rule.first << ": {";
        std::string seprator = "";
        for (const auto& style : rule.second.styles)
        {
            s << seprator << style.first << ": " << style.second;
            seprator = ", ";
        }
        s << "}" << std::endl;
    }
    std::cout << s.str();
}
