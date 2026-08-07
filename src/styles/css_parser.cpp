#include "css_parser.h"

#include "css_parser_exception.h"
#include "helpers/string_helpers.h"
#include "styles/descendant_selector.h"
#include "styles/selector.h"
#include "styles/tag_selector.h"

#include <algorithm>

CSSParser::CSSParser(std::string text)
    : m_text(text),
      m_i(0)
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
    size_t start = m_i;
    while (m_i < m_text.size())
    {
        std::vector<char> specials = {'#', '-', '.', '%'};
        if (std::isalnum(m_text[m_i]) || 
            std::ranges::contains(specials, m_text[m_i]))
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
    while (m_i < m_text.size() && m_text[m_i] != '}')
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

std::unique_ptr<Selector> CSSParser::selector()
{
    auto w = toLowercase(word());
    std::unique_ptr<Selector> out = std::make_unique<TagSelector>(w);
    whitespace();
    while (m_i < m_text.size() && m_text[m_i] != '{')
    {
        w = toLowercase(word());
        auto descendant = std::make_unique<TagSelector>(w);
        out = std::make_unique<DescendantSelector>(std::move(out), std::move(descendant));
        whitespace();
    }
    return out;
}

CSSRules CSSParser::parse()
{
    CSSRules rules;

    while (m_i < m_text.size())
    {
        try 
        {
            whitespace();
            auto selector = this->selector();
            literal('{');
            whitespace();
            auto body = this->body();
            literal('}');
            rules[std::move(selector)] = body;
        } 
        catch (const CSSParserException& e) 
        {
            // We failed to parse the selector, so skip the whole rule.
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

/// Helper function that sorts a map of CSSRules by priority while 
/// maintaining the original order as much as possible (AKA cascade 
/// ordering), returning the results as a vector of pairs.
SortedCSSRules sort(const CSSRules& rules)
{
    SortedCSSRules sortedRules;
    for (const auto& rule : rules)
    {
        if (auto tagSelector = dynamic_cast<TagSelector*>(rule.first.get()))
        {
            sortedRules.push_back({
                std::make_unique<TagSelector>(tagSelector->getName()), 
                rule.second
            });
        }
        else if (auto s = dynamic_cast<DescendantSelector*>(rule.first.get()))
        {
            if (s->descendant == nullptr && s->ancestor == nullptr) {
                continue;
            }

            sortedRules.push_back({
                std::make_unique<DescendantSelector>(
                    std::move(s->ancestor), 
                    std::move(s->descendant)
                ), 
                rule.second
            });
        }
    }

    std::stable_sort(sortedRules.begin(), sortedRules.end(), 
        [](const auto& lhs, const auto& rhs) 
        {
            return lhs.first->getPriority() > rhs.first->getPriority();
        }
    );
   
    return sortedRules;
}
