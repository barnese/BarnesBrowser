#pragma once

#include "helpers/ssl_helper.h"
#include "url.h"

#include <optional>
#include <string>
#include <vector>

/// Handles the lifecycle of HTTP connections, including sending requests,
/// receiving responses and determining whether the connection can be reused.
class HttpConnection
{
public:
    HttpConnection();

    HttpConnection(const HttpConnection&) = delete;
    HttpConnection& operator=(const HttpConnection&) = delete;

    void close();
    void sendRequest(const URL& url, std::string request);
    void receiveResponse();
    std::string readHeaders();
    std::vector<std::byte> readBody(std::optional<size_t> contentLength = std::nullopt);
 
private:
    void connect(const URL& url);
    bool canReuse(const URL& url);
    void receive();
    std::vector<std::byte> read(size_t count);

    int m_sock = -1;
    bool m_useSSL = false;
    SSLHelper m_ssl;
    std::optional<URL> m_lastURL;
    std::vector<std::byte> m_receiveBuffer;
};
