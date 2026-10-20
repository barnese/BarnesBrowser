#include "header_parser_helpers.h"

#include <regex>
#include <sstream>

/// Parses the status code from the given header text.
std::string parseStatusCode(std::string headerText)
{
    // Parse the status line.
    std::istringstream stream(headerText);
    std::string statusLine;
    std::getline(stream, statusLine);
    std::istringstream statusStream(statusLine);
    std::string version;
    std::string statusCode;
    statusStream >> version;
    statusStream >> statusCode;
    return statusCode;
}

std::optional<CachePolicy> parseCacheControl(std::string headerText)
{
    if (headerText.contains("no-store"))
    {
        return CachePolicy { true, std::nullopt };
    }

    // For now, just support max-age.
    std::regex pattern(R"(max-age=(\d+))", std::regex::icase);
    std::smatch match;

    if (std::regex_search(headerText, match, pattern))
    {
        auto capturedValue = std::stoi(match[1]);
        std::chrono::seconds maxAge(capturedValue);
        return CachePolicy { false, maxAge };
    }

    return std::nullopt;
}
