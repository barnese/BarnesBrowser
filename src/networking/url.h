#pragma once

#include "helpers/ssl_helper.h"
#include "url_components.h"
#include "url_response.h"

#include <span>
#include <string>
#include <optional>
#include <vector>

class URL
{
public:
    URL(const std::string& url);
    
    /// @brief Performs a request for this URL.
    /// @param referrer - optionally indicates which site the request is being made from.
    /// @param payload - optionally allows sending a payload via POST request.
    /// @returns a URLResponse object, if successful.
    URLResponse request(std::optional<URL> referrer, std::optional<std::string> payload=std::nullopt) const;
    
    /// Converts a relative URL into a full URL based on the current URL.
    URL resolve(std::string url) const;

    bool operator==(const URL& other) const;

    /// Gets the URL as a string with the path stripped off.
    std::string origin() const;

    std::string describe(bool showPort = true) const;

    std::string host() const { return m_urlComponents.host; };

    UrlComponents components() const { return m_urlComponents; };

private:
    void sendRequest(SSLHelper* ssl, int sock, std::string request) const;
    
    /// Receives the response from a URL request.
    /// @returns binary response data
    std::vector<std::byte> receiveResponse(SSLHelper* ssl, int sock) const;

    /// Reads the headers from a span that points to some binary data.
    /// @param pos - reference to a position variable which helps
    /// determine where the headers end and the body begins
    /// @returns headers as a string
    std::string readHeaders(std::span<const std::byte> data, size_t& pos) const;

    void addCookieToRequest(std::optional<URL> referrer, std::ostringstream& out);

    UrlComponents m_urlComponents;
};
