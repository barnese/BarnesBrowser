#pragma once

#include "helpers/case_insensitive_less.h"
#include "helpers/ssl_helper.h"

#include <map>
#include <string>
#include <optional>

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

struct URLResponse
{
    std::map<std::string, std::string, CaseInsensitiveLess> headers;
    std::string statusCode;
    std::optional<std::string> errorMessage;
    std::string body;
};

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
        std::string receiveResponse(SSLHelper* ssl, int sock) const;
        void addCookieToRequest(std::optional<URL> referrer, std::ostringstream& out);

        UrlComponents m_urlComponents;
};