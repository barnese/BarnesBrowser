#include "html_parser.h"

#include "element.h"
#include "helpers/string_helpers.h"
#include "text.h"

#include <algorithm>
#include <iostream>

HTMLParser::HTMLParser(std::string body)
    : m_body(body)
{
}

std::unique_ptr<Node> HTMLParser::parse()
{
    std::string buffer = "";
    bool inTag = false;

    for (char c : m_body)
    {
        if (c == '<')
        {
            inTag = true;
            if (!buffer.empty())
            {
                addText(buffer);
            }
            buffer = "";
        }
        else if (c == '>')
        {
            inTag = false;
            addTag(buffer);
            buffer = "";
        }
        else
        {
            // Replace line breaks with spaces.
            if (c == '\n')
            {
                buffer += ' ';
            } 
            else 
            {
                buffer += c;
            }
        }        
    }

    if (!inTag && !buffer.empty())
    {
        addText(buffer);
    }

    return finish();
}

Node* HTMLParser::getLastUnfinishedNode()
{
    if (m_unfinished.empty())
    {
        return nullptr;
    }

    return m_unfinished.back().get();
}

void HTMLParser::addTag(const std::string& text)
{
    auto components = getTagAttributes(text);
    auto tag = components.tag;
    auto attributes = components.attributes;

    // Ignore special tags like doctype.
    if (tag.starts_with('!'))
    {
        return;
    }

    addImplicitTags(tag);

    if (tag.starts_with('/'))
    {
        // The very last tag is an edge case. Since there is
        // no unfinished node to add it to, just return.
        if (m_unfinished.size() == 1)
        {
            return;
        }

        // A close tag finishes the last unfinished node by
        // adding it to the previous unfinished node in the list.
        // For example, say the parser is on a </b> close tag: 
        // ...<p><b>hi</b>
        // We pop <b>, get <p>, and add <b> as a child of <p>.
        auto node = std::move(m_unfinished.back());
        m_unfinished.pop_back();
        const auto& parent = m_unfinished.back();
        parent->addChild(std::move(node));
    }
    else if (std::ranges::contains(SELF_CLOSING_TAGS, tag))
    {
        // Add a closing tag for self-closing tags, such as link or br.
        auto parent = getLastUnfinishedNode();
        auto node = std::make_unique<Element>(tag, parent);
        node->attributes = attributes;
        parent->addChild(std::move(node));
    }
    else
    {
        // An open tag adds an unfinished node to the end of the list.
        // If the list is empty, parent will be a nullptr.
        auto parent = getLastUnfinishedNode();
        auto node = std::make_unique<Element>(tag, parent);
        node->attributes = attributes;
        m_unfinished.push_back(std::move(node));
    } 
}

void HTMLParser::addText(const std::string& text)
{
    // Ignore whitespace.
    if (trim(text) == "") 
    {
        return;
    }

    addImplicitTags();

    // Adds the text as a child node to the last unfinished node.
    const auto& parent = m_unfinished.back();
    auto node = std::make_unique<Text>(text);
    parent->addChild(std::move(node));
}

/// Adds some missing required tags implicitly, such as an omitted <html> tag.
/// This is a very basic implementation and needs to be expanded in the future.
void HTMLParser::addImplicitTags(const std::optional<std::string>& tag)
{
    while (true)
    {
        std::vector<std::string> openTags;
        
        for (const auto& node : m_unfinished)
        {
            if (auto element = dynamic_cast<const Element*>(node.get())) 
            {
                openTags.push_back(element->tag);
            }
        }

        // Add an implicit <html> tag if the first tag in the document
        // is something other than an <html> tag.
        if (openTags.empty() && tag != "html")
        {
            addTag("html");
        }
        else if (openTags.size() == 1 
            && openTags[0] == "html" 
            && !std::ranges::contains(std::vector{"head", "body", "/html"}, tag))
        {
            // Add an implicit <head> or <body> tag. 
            if (std::ranges::contains(HEAD_TAGS, tag))
            {
                addTag("head");
            }
            else
            {
                addTag("body");
            }
        }
        else if (openTags.size() == 2 
            && openTags[0] == "html" 
            && openTags[1] == "head"
            && tag != "/head"
            && !std::ranges::contains(HEAD_TAGS, tag))
        {
            addTag("/head");
        }
        else 
        {
            break;
        }
    }
}

TagAttributes HTMLParser::getTagAttributes(const std::string& text)
{
    auto parts = split(text, " ", false);
    auto tag = toLowercase(parts[0]);
    std::map<std::string, std::string> attributes;

    for (size_t i = 1; i < parts.size(); ++i)
    {
        auto attrpair = parts[i];
        if (attrpair == "/")
        {
            // Ignore a slash at the end of a self closing tag.
            continue;
        }
        else if (attrpair.contains("="))
        {
            // Handle the case where the key/value pair is separated by an = sign.
            auto pair = split(attrpair, "=");
            auto key = pair[0];
            auto value = pair[1];

            // If the value is quoted, strip the quotes.
            if (value.size() > 2 && (value[0] == '\'' || value[0] == '"')) 
            {
                value = value.substr(1, value.size() - 2);
            }

            attributes[toLowercase(key)] = value;
        }
        else 
        {
            // Handle the case where there is no value.
            attributes[toLowercase(attrpair)] = "";
        }
    }

    return TagAttributes(tag, attributes);
}

std::unique_ptr<Node> HTMLParser::finish()
{
    // Turn an incomplete tree into a complete tree
    // by finishing any unfinished nodes.
    while (m_unfinished.size() > 1)
    {
        addImplicitTags();
        auto node = std::move(m_unfinished.back());
        m_unfinished.pop_back();
        const auto& parent = m_unfinished.back();
        parent->addChild(std::move(node));
    }
    auto node = std::move(m_unfinished.back());
    m_unfinished.pop_back();
    return node;
}

void HTMLParser::printTree(Node* node, int indent)
{
    std::string spaces;
    for (int i = 0; i < indent; ++i) 
    {
        spaces += " ";
    }

    std::cout << spaces << node->describe() << "\n";

    for (const auto& node : node->getChildren())
    {
        printTree(node.get(), indent + 2);
    }
}
