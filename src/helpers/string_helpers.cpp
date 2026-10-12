#include "string_helpers.h"
#include <algorithm>
#include <regex>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <openssl/evp.h>

std::vector<std::string> split(const std::string& text, const std::string& delimiter, bool firstOnly)
{
    std::vector<std::string> result;

    std::size_t start = 0;
    std::size_t end;

    while ((end = text.find(delimiter, start)) != std::string::npos)
    {
        result.push_back(text.substr(start, end - start));
        start = end + delimiter.length();

        if (firstOnly) 
        {
            break;
        }
    }

    result.push_back(text.substr(start));

    return result;
}

std::vector<std::string> splitLast(const std::string& text, const std::string& delimiter)
{
    size_t pos = text.rfind(delimiter);
 
    if (pos != std::string::npos)
    {
        std::string first = text.substr(0, pos);
        std::string last = text.substr(pos + 1);
        return { first, last };
    }

    return { text };
}

std::vector<std::string> splitBySpaceQuoted(const std::string& text)
{
    std::vector<std::string> result;
    std::string buffer;
    bool inSingleQuotes = false;
    bool inDoubleQuotes = false;

    for (char c : text)
    {
        if (c == '\'')
        {
            inSingleQuotes = !inSingleQuotes;
        }

        if (c == '"')
        {
            inDoubleQuotes = !inDoubleQuotes;
        }

        if (c == ' ' || c == '\n' || c == '\r')
        {
            if (!inSingleQuotes && !inDoubleQuotes)
            {
                result.push_back(buffer);
                buffer = "";
            }
            else 
            {
                buffer += c;
            }
        }
        else 
        {
            buffer += c;
        }
    }

    if (!inSingleQuotes && !inDoubleQuotes && !buffer.empty())
    {
        result.push_back(buffer);
    }

    return result;
}

std::string trim(std::string text) 
{
    return trimRight(trimLeft(text));
}

std::string trimLeft(std::string text)
{
    std::size_t start = text.find_first_not_of(" \t\n\r\f\v");

    if (start == std::string::npos) 
        return "";

    return text.substr(start);
}

std::string trimRight(std::string text)
{
    std::size_t end = text.find_last_not_of(" \t\n\r\f\v");
    if (end == std::string::npos)
        return "";
    return text.substr(0, end + 1);
}

std::string toLowercase(std::string text)
{
    std::ranges::transform(text, text.begin(),
        [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        });

    return text;
}

std::string removeComments(std::string text)
{
    // Replace /* */ style comments with an empty string.
    std::regex regex("/\\*(.|[\r\n])*?\\*/");
    return std::regex_replace(text, regex, "");
}

/// Converts a string percentage, such as "95%", to a 
/// float value between 0 and 1.
float percentToFloat(std::string percentString)
{
    if (!percentString.empty() && percentString.back() == '%')
    {
        percentString.pop_back();
    }

    return std::stof(percentString) / 100.0f;
}

/// Converts a pixel string to a float. For example: "12px" returns 12.
float pixelsToFloat(std::string pixelString)
{
    if (pixelString.size() > 2 && pixelString.substr(pixelString.size() - 2) == "px")
    {
        auto numericPart = pixelString.substr(0, pixelString.size() - 2);
        return std::stof(numericPart);
    }
    return std::stof(pixelString);
}

double pixelsToDouble(std::string pixelString)
{
    if (pixelString.size() > 2 && pixelString.substr(pixelString.size() - 2) == "px")
    {
        auto numericPart = pixelString.substr(0, pixelString.size() - 2);
        return std::stod(numericPart);
    }
    return std::stod(pixelString);
}

std::optional<int> pixelsToInt(std::string pixelString)
{
    try 
    {
        if (pixelString.size() > 2 && pixelString.substr(pixelString.size() - 2) == "px")
        {
            auto numericPart = pixelString.substr(0, pixelString.size() - 2);
            return std::stoi(numericPart);
        }
        else if (!pixelString.empty())
        {
            return std::stoi(pixelString);
        }
    }
    catch (...) 
    {
        std::cerr << "[StringHelpers] pixelToInt() failed for: " << pixelString << std::endl;
    }

    return std::nullopt;
}

/// Percent URL encodes a string.
std::string urlEncode(const std::string& value) 
{
    std::ostringstream escaped;
    escaped << std::hex << std::uppercase;

    for (char c : value) 
    {
        // Keep alphanumeric and other safe characters intact
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~') 
        {
            escaped << c;
        } 
        else 
        {
            // Percent-encode everything else
            escaped 
                << '%' 
                << std::setw(2) 
                << std::setfill('0') 
                << static_cast<int>(static_cast<unsigned char>(c));
        }
    }

    return escaped.str();
}

std::optional<std::string> decodeBase64(std::string_view input) {
    if (input.size() % 4 != 0)
        return std::nullopt;

    std::string output(3 * (input.size() / 4), '\0');

    int length = EVP_DecodeBlock(
        reinterpret_cast<unsigned char*>(output.data()),
        reinterpret_cast<const unsigned char*>(input.data()),
        static_cast<int>(input.size())
    );

    if (length < 0)
        return std::nullopt;

    // Remove bytes added because of Base64 padding.
    if (!input.empty() && input.back() == '=') --length;
    if (input.size() > 1 && input[input.size() - 2] == '=') --length;

    output.resize(length);
    return output;
}
