#pragma once

#include "url_components.h"

#include <string>

class URL
{
public:
    URL(const std::string& url);

    /// Converts a relative URL into a full URL based on the current URL.
    URL resolve(std::string url) const;

    bool operator==(const URL& other) const;

    /// Gets the URL as a string with the path stripped off.
    std::string origin() const;

    std::string host() const { return m_urlComponents.host; };
    UrlComponents components() const { return m_urlComponents; };
    std::string describe(bool showPort = true) const;

private:
    UrlComponents m_urlComponents;
};
