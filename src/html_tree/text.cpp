#include "text.h"

Text::Text(const std::string& text)
    : text(text)
{
    isFocused = false;
}

std::string Text::getText() const
{
    return text;
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
