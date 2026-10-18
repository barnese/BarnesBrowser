#include "http_connection.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

HttpConnection::HttpConnection()
{
}

/// Looks up an address given a host and port.
/// Creates a socket and connects to that address.
int lookupDNSAndConnect(std::string host, uint16_t port)
{
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* result = nullptr;

    // Do a DNS lookup to resolve the host using TCP.
    int address = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result);

    if (address != 0)
        throw std::runtime_error("DNS lookup failed!");

    // Create the socket.
    int sock = socket(result->ai_family, 
                      result->ai_socktype,
                      result->ai_protocol);

    if (sock == 0)
        throw std::runtime_error("Failed to create the socket!");

    if (connect(sock, result->ai_addr, result->ai_addrlen) == -1) 
        throw std::runtime_error("Error connecting to the host!");

    return sock;
}

bool HttpConnection::canReuse(const URL& url)
{
    // Determine is we can reuse the existing connection.
    return m_sock >= 0 && 
        m_lastURL && 
        m_lastURL->components().host == url.components().host &&
        m_lastURL->components().port == url.components().port &&
        m_lastURL->components().scheme == url.components().scheme;
}

void HttpConnection::connect(const URL& url)
{
    if (!canReuse(url))
    {
        std::cout << "[HttpConnection] resetting connection\n";
        close();

        m_sock = lookupDNSAndConnect(url.components().host, url.components().port);

        if (url.components().scheme == "https")
        {
            m_useSSL = true;
            m_ssl.connect(m_sock, url.components().host);
        }
    }
}

void HttpConnection::close()
{
    if (m_useSSL)
    {
        m_ssl.shutdown();
    }

    ::close(m_sock);
    m_sock = 0;
    m_lastURL = std::nullopt;
    m_receiveBuffer.clear();
}

void HttpConnection::sendRequest(const URL& url, std::string request)
{
    connect(url);

    size_t totalSent = 0;
    while (totalSent < request.size())
    {
        ssize_t sent;
        if (m_useSSL)
        {
            sent = m_ssl.write(request.data() + totalSent, request.size() - totalSent);
        }
        else
        {
            sent = send(m_sock, request.data() + totalSent, request.size() - totalSent, 0);
        }

        if (sent <= 0)
        {
            std::cout << "[HttpConnection] there was an error sending the request!\n";
            break;
        }

        totalSent += sent; 
    }

    m_lastURL = url;
}

void HttpConnection::receiveResponse()
{
    std::array<std::byte, 4096> chunk;

    while (true)
    {
        ssize_t received;

        if (m_useSSL)
        {
            received = m_ssl.read(chunk.data(), chunk.size());
        }
        else
        {
            received = recv(m_sock, chunk.data(), chunk.size(), 0);
        }

        if (received > 0)
        {
            m_receiveBuffer.insert(m_receiveBuffer.end(), chunk.begin(), chunk.begin() + received);
        }
        else if (received == 0)
        {
            break;
        }
        else
        {
            throw std::runtime_error("There was an error receiving response");
        }
    }
}

void HttpConnection::receive()
{
    std::array<std::byte, 4096> chunk;

    ssize_t bytesReceived;

    if (m_useSSL)
    {
        bytesReceived = m_ssl.read(chunk.data(), chunk.size());
    }
    else 
    {
        bytesReceived = recv(m_sock, chunk.data(), chunk.size(), 0);
    }

    if (bytesReceived <= 0)
    {
        throw std::runtime_error("Connection closed or read failed");
    }
        
    m_receiveBuffer.insert(m_receiveBuffer.end(), chunk.begin(), chunk.begin() + bytesReceived);
}

std::vector<std::byte> HttpConnection::read(size_t count)
{
    while (m_receiveBuffer.size() < count)
    {
        receive();
    }

    std::vector<std::byte> result(m_receiveBuffer.begin(), m_receiveBuffer.begin() + count);

    m_receiveBuffer.erase(m_receiveBuffer.begin(), m_receiveBuffer.begin() + count);

    return result;
}

std::string HttpConnection::readHeaders()
{
    constexpr std::array delimiter
    {
        std::byte{'\r'},
        std::byte{'\n'},
        std::byte{'\r'},
        std::byte{'\n'}
    };

    while (true)
    {
        // Search for the header delimiter in the current buffer.
        // If we find it, return the buffer containing the headers.
        auto it = std::search(
            m_receiveBuffer.begin(), 
            m_receiveBuffer.end(), 
            delimiter.begin(), 
            delimiter.end()
        );

        if (it != m_receiveBuffer.end())
        {
            auto end = it + delimiter.size();

            std::vector<std::byte> headers(m_receiveBuffer.begin(), end);

            m_receiveBuffer.erase(m_receiveBuffer.begin(), end);

            return reinterpret_cast<const char*>(headers.data());
        }

        receive();
    }
}

std::vector<std::byte> HttpConnection::readBody(std::optional<size_t> contentLength)
{
    int length = contentLength ? *contentLength : m_receiveBuffer.size();

    return read(length);
}
