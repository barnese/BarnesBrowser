#pragma once

#include <cstdint>
#include <optional>
#include <string>

struct UrlComponents 
{
    std::string scheme;
    std::string host;
    std::string path;
    uint16_t port;

    /// Support for data URLs
    bool base64 = false;
    std::optional<std::string> mediaType = std::nullopt;
    std::optional<std::string> data = std::nullopt;

    bool viewSource = false;
    bool aboutBlank = false;

    bool operator==(const UrlComponents& other) const
    {
        return scheme == other.scheme &&
            host == other.host &&
            path == other.path &&
            port == other.port &&
            base64 == other.base64 &&
            mediaType == other.mediaType &&
            data == other.data;
    }
};

/// Helper that gets the URL components for the given URL.
UrlComponents getUrlComponents(std::string url);
