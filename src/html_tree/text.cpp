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
    return text;
}
