#pragma once

#include "node.h"

#include <map>
#include <optional>
#include <string>
#include <QImage>

/// Represents a tag or "element" node in a tree.
class Element : public Node
{
public:
    Element(const std::string& tag, Node* parent = nullptr);
    std::string describe() const override;

    std::string tag;

    std::unique_ptr<QImage> image;

    std::map<std::string, std::string> attributes;

    std::string findAttribute(const std::string& name, const std::string& defaultValue) const   
    {
        auto it = attributes.find(name);
        return (it != attributes.end()) ? it->second : defaultValue;
    }

    std::optional<std::string> findAttribute(const std::string& name) const
    {
        auto it = attributes.find(name);
        if (it != attributes.end())
            return it->second;
        else
            return std::nullopt;
    }
};
