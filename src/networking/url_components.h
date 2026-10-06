#pragma once

#include <string>

struct UrlComponents 
{
    std::string scheme;
    std::string host;
    std::string path;
    uint16_t port;

    bool operator==(const UrlComponents& other) const
    {
        return scheme == other.scheme &&
            host == other.host &&
            path == other.path &&
            port == other.port;
    }
};

/// Helper that gets the URL components for the given URL.
UrlComponents getUrlComponents(std::string url);
