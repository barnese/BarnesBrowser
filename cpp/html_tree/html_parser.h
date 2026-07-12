#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "node.h"

struct TagAttributes
{
    std::string tag;
    std::map<std::string, std::string> attributes;
};

/// Parses HTML text into an HTML tree.
/// The HTML tree contains Element nodes for tags and Text nodes for text.
class HTMLParser
{
public:
    HTMLParser(std::string body);
    std::unique_ptr<Node> parse();
    void printTree(Node* node, int indent = 0);

    const std::vector<std::string> HEAD_TAGS = 
    {
        "base", "basefont", "bgsound", "noscript",
        "link", "meta", "title", "style", "script"
    };

    const std::vector<std::string> SELF_CLOSING_TAGS = 
    {
        "area", "base", "br", "col", "embed", "hr", "img", "input",
        "link", "meta", "param", "source", "track", "wbr"
    };

private:
    void addTag(const std::string& tag);
    void addText(const std::string& text);
    void addImplicitTags(const std::optional<std::string>& tag = std::nullopt);
    TagAttributes getTagAttributes(const std::string& text);
    std::unique_ptr<Node> finish();
    Node* getLastUnfinishedNode();

    std::string m_body;
    std::vector<std::unique_ptr<Node>> m_unfinished;
};
