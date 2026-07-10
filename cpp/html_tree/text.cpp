#include "text.h"

Text::Text(const std::string& text)
    : text(text)
{
}

std::string Text::getText() const
{
    return text;
}

std::string Text::describe() const
{
    return text;
}
