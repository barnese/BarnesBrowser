#include "url.h"
#include "helpers/string_helpers.h"
#include "constants.h"
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

/// Gets the URL components for the given URL.
UrlComponents getUrlComponents(std::string url)
{
    UrlComponents components;
    auto parts = split(url, "://");
    components.scheme = parts[0];
    
    auto urlParts = split(parts[1], "/");
    components.host = urlParts[0];
    components.path = "/" + urlParts[1];

    if (components.host.contains(':')) 
    {
        auto hostParts = split(components.host, ":");
        components.host = hostParts[0];
        components.port = std::stoi(hostParts[1]);
    } 
    else
    {
        if (components.scheme == "https")
            components.port = 443;
        else 
            components.port = 80;
    } 

    return components;
}

/// Constructor
URL::URL(const std::string& url)
{
    m_urlComponents = getUrlComponents(url);
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

/// Formats a HTTP GET request.
std::string formatRequest(std::string path, std::string host)
{
    std::ostringstream out;
    out << "GET " << path << " HTTP/1.0\r\n";
    out << "Host: " << host << "\r\n";
    out << "\r\n";
    return out.str();
}

/// Sends a URL request.
void URL::sendRequest(SSLHelper* ssl, int sock, std::string request) const
{
    size_t totalSent = 0;
    while (totalSent < request.size())
    {
        ssize_t sent;
        if (m_urlComponents.scheme == "https")
            sent = ssl->write(request.data() + totalSent, request.size() - totalSent);
        else
            sent = send(sock, request.data() + totalSent, request.size() - totalSent, 0);

        if (sent <= 0)
        {
            std::cout << "there was an error sending the request!\n";
            break;
        }

        totalSent += sent; 
    }

    std::cout << "Sent request: " << totalSent << " bytes.\n";
}

/// Receives the response from a URL request.
std::string URL::receiveResponse(SSLHelper* ssl, int sock) const
{
    std::string response;
    char buffer[4096];
    while (true)
    {
        ssize_t received;
        if (m_urlComponents.scheme == "https")
            received = ssl->read(buffer, sizeof(buffer));
        else
            received = recv(sock, buffer, sizeof(buffer), 0);

        if (received > 0)
        {
            response.append(buffer, received);
        }
        else if (received == 0)
        {
            break;
        }
        else
        {
            std::cout << "There was an error receiving response!\n";
            break;
        }
    }

    return response;
}

/// Determines if the status code is a redirect.
bool isRedirect(int statusCode)
{
    return statusCode >= 301 && statusCode <= 308;
}

/// Parses the status code from the given header text.
std::string getStatusCode(std::string headerText)
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

/// @brief Parses header text from a HTML response.
/// @param headerText text from the header
/// @return A map of header keys and values
std::map<std::string, std::string> parseHeaders(std::string headerText)
{
    std::istringstream stream(headerText);
    std::string line;
    std::map<std::string, std::string> headers;

    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        size_t colon = line.find(':');

        if (colon == std::string::npos)
            continue;
        
        std::string name = line.substr(0, colon);
        std::string value = line.substr(colon + 1);

        headers[name] = trim(value);
    }

    // Make sure these headers are not found.
    assert(headers.find("transfer-encoding") == headers.end());
    assert(headers.find("content-encoding") == headers.end());

    return headers;
}

/// @brief Performs a request for this URL.
/// @return Returns the body of the response, if successful.
std::string URL::request() const
{
    UrlComponents urlComponents = m_urlComponents;

    // Loop to handle URLs that respond with a redirect responses, 
    // but only up to 10 times to avoid infinite redirect loops.
    for (int redirectCount = 0; redirectCount < 10; ++redirectCount)
    {
        int sock = lookupDNSAndConnect(urlComponents.host, urlComponents.port);
        std::string request = formatRequest(urlComponents.path, urlComponents.host);

        SSLHelper ssl;
        if (urlComponents.scheme == "https")
            ssl.connect(sock, urlComponents.host);

        sendRequest(&ssl, sock, request);
        std::string response = receiveResponse(&ssl, sock);

        if (urlComponents.scheme == "https")
            ssl.shutdown();
        close(sock);

        // Get the header and body text from the response.
        size_t headerEnd = response.find(Constants::HTML_HEADER_SEPARATOR);
        std::string header = response.substr(0, headerEnd);
        std::string body = response.substr(headerEnd + Constants::HTML_HEADER_SEPARATOR.size());
        std::string statusCode = getStatusCode(header);
        auto headers = parseHeaders(header);

        if (!isRedirect(std::stoi(statusCode)))
        {
            return body;
        }

        // Create a new request for the given redirect location.
        // TODO: Solidify this to handle relative paths.
        auto location = headers["Location"];
        urlComponents = getUrlComponents(location);
    }

    return "Error: Too many redirects!";
}
