#pragma once

#include "http_cache.h"
#include "http_connection.h"
#include "url.h"
#include "url_components.h"
#include "url_response.h"

#include <optional>
#include <span>

/// Responsible for loading resources like web pages, images, etc.
class ResourceLoader
{
public:
    ResourceLoader(bool keepAlive);
    ~ResourceLoader();

    /// @brief Performs a request for the given URL.
    /// @param referrer - optionally indicates which site the request is being made from.
    /// @param payload - optionally allows sending a payload via POST request.
    /// @returns a URLResponse object, if successful.
    URLResponse request(URL url, std::optional<URL> referrer, std::optional<std::string> payload=std::nullopt);
    
private:
    URLResponse requestFromFile(const std::string& path, bool viewSource) const;
    URLResponse requestFromData(const UrlComponents& components) const;
    URLResponse requestFromNetwork(URL url, const std::optional<URL>& referrer, std::optional<std::string> payload=std::nullopt);

    /// Reads the headers from a span that points to some binary data.
    /// @param pos - reference to a position variable which helps
    /// determine where the headers end and the body begins
    /// @returns headers as a string
    std::string readHeaders(std::span<const std::byte> data, size_t& pos) const;

    void addCookieToRequest(std::optional<URL> referrer, std::ostringstream& out);

    void debugOutRequest(const std::string& request);

    bool m_keepAlive;
    std::unique_ptr<HttpConnection> m_connection;
    HttpCache m_httpCache;
};
