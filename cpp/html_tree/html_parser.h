#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "node.h"

using TagAttributes = std::map<std::string, std::map<std::string, std::string>>;

/// Parses HTML text into an HTML tree.
/// The HTML tree contains Element nodes for tags and Text nodes for text.
class HTMLParser
{
public:
    HTMLParser(std::string body);
    std::unique_ptr<Node> parse();
    void printTree(Node* node, int indent = 0);

    const std::vector<std::string> SELF_CLOSING_TAGS = 
    {
        "area", "base", "br", "col", "embed", "hr", "img", "input",
        "link", "meta", "param", "source", "track", "wbr"
    };

protected:
    void addTag(std::string tag);
    void addText(std::string text);
    std::unique_ptr<Node> finish();
    Node* getLastUnfinishedNode();
    TagAttributes getTagAttributes(std::string text);

private:
    std::string m_body;
    std::vector<std::unique_ptr<Node>> m_unfinished;
};
