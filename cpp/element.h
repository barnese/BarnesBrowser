#pragma once

#include <string>

class Element
{
public:
    Element(std::string text, bool isTag);
    std::string getText() const;
    bool isTag() const;

private:
    std::string m_text;
    bool m_isTag;
};
