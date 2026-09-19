#include "text.h"

Text::Text(const std::string& text, Node* parent)
    : text(text)
{
    isFocused = false;
    this->parent = parent;
}

std::string Text::getText() const
{
    return text;
}

void Text::appendText(std::string text) 
{
    this->text += text;
}

std::string Text::describe() const
{
    std::string s;

    if (text.size() > 50)
        s = text.substr(0, 50) + "...";
    else
        s = text;

    return "\"" + s + "\"";
}
