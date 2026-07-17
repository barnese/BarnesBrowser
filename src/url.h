#pragma once

#include "helpers/ssl_helper.h"
#include <string>

struct UrlComponents {
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

class URL
{
    public:
        URL(const std::string& url);
        std::string request() const;
        URL resolve(std::string url);
        bool operator==(const URL& other) const;
    
    private:
        void sendRequest(SSLHelper* ssl, int sock, std::string request) const;
        std::string receiveResponse(SSLHelper* ssl, int sock) const;

        UrlComponents m_urlComponents;
};