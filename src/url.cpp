#include "url.h"

#include "helpers/string_helpers.h"
#include "constants.h"
#include "globals.h"

#include <cassert>
#include <iostream>
#include <optional>
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
    if (!url.empty())
    {
        m_urlComponents = getUrlComponents(url);
    }
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

void addCookieToRequest(std::string host, std::string method, std::optional<URL> referrer, std::ostringstream& out)
{
    if (Globals::cookieJar.contains(host))
    {
        bool allowCookie = true;
        auto [cookie, params] = Globals::cookieJar[host];
        auto it = params.find("samesite");
        if (referrer.has_value() && it != params.end() && it->second == "lax")
        {
            if (method != "GET")
            {
                allowCookie = host == referrer->host();
            }
        }
        if (allowCookie)
        {
            out << "Cookie: " << cookie << "\r\n";
        }
    }
}

/// Formats a HTTP GET request.
std::string formatGetRequest(std::string path, std::string host, std::optional<URL> referrer)
{
    std::ostringstream out;
    out << "GET " << path << " HTTP/1.1\r\n";
    out << "Host: " << host << "\r\n";
    out << "Connection: close\r\n";
    out << "User-Agent: BBrowser/1.0\r\n";
    addCookieToRequest(host, "GET", referrer, out);
    out << "\r\n";
    return out.str();
}

std::string formatPostRequest(std::string path, std::string host, std::optional<URL> referrer, std::string payload)
{
    std::ostringstream out;
    out << "POST " << path << " HTTP/1.1\r\n";
    out << "Host: " << host << "\r\n";
    out << "Connection: close\r\n";
    out << "User-Agent: BBrowser/1.0\r\n";
    out << "Content-Length: " << payload.size() << "\r\n";
    addCookieToRequest(host, "POST", referrer, out);
    out << "\r\n";
    out << payload;
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
std::map<std::string, std::string, CaseInsensitiveLess> parseHeaders(std::string headerText)
{
    std::istringstream stream(headerText);
    std::string line;
    std::map<std::string, std::string, CaseInsensitiveLess> headers;

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

void parseAndStoreCookie(std::string host, std::string cookie)
{
    std::map<std::string, std::string> params;

    if (cookie.contains(";"))
    {
        auto cookieParts = split(cookie, ";");
        cookie = cookieParts[0];
        auto rest = split(cookieParts[1], ";", false);
        for (const auto& param : rest)
        {
            std::string key;
            std::string value;

            if (param.contains("="))
            {
                auto parts = split(param, "=");
                key = parts[0];
                value = parts[1];
            }
            else 
            {
                key = param;
                value = "true";        
            }

            params[trim(toLowercase(key))] = toLowercase(value);
        }
    }

    Globals::cookieJar[host] = {cookie, params};
}

URLResponse URL::request(std::optional<URL> referrer, std::optional<std::string> payload) const
{
    URLResponse response;

    UrlComponents urlComponents = m_urlComponents;

    // Loop to handle URLs that respond with a redirect responses, 
    // but only up to 10 times to avoid infinite redirect loops.
    for (int redirectCount = 0; redirectCount < 10; ++redirectCount)
    {
        int sock = lookupDNSAndConnect(urlComponents.host, urlComponents.port);
        std::string request;
        if (payload == std::nullopt)
        {
            request = formatGetRequest(urlComponents.path, urlComponents.host, referrer);
        }
        else
        {
            request = formatPostRequest(urlComponents.path, urlComponents.host, referrer, payload.value());
        }

        SSLHelper ssl;
        if (urlComponents.scheme == "https")
            ssl.connect(sock, urlComponents.host);

        sendRequest(&ssl, sock, request);

        std::string responseString = receiveResponse(&ssl, sock);

        if (urlComponents.scheme == "https")
        {
            ssl.shutdown();
        }
        close(sock);

        // Get the header and body text from the response.
        size_t headerEnd = responseString.find(Constants::HTML_HEADER_SEPARATOR);
        std::string header = responseString.substr(0, headerEnd);
        response.body = responseString.substr(headerEnd + Constants::HTML_HEADER_SEPARATOR.size());
        response.statusCode = getStatusCode(header);
        response.headers = parseHeaders(header);

        if (response.headers.contains("set-cookie"))
        {
            auto cookie = response.headers["set-cookie"];
            parseAndStoreCookie(urlComponents.host, cookie);
        }

        if (!isRedirect(std::stoi(response.statusCode)))
        {
            return response;
        }

        // Create a new request for the given redirect location.
        auto location = response.headers["Location"];
        auto fullUrl = resolve(location);
        urlComponents = fullUrl.components();
        std::cout << "Redirecting to: " << fullUrl.describe(false) << std::endl;
    }

    response.errorMessage = "Too many redirects!";
    return response;
}

URL URL::resolve(std::string url) const
{
    if (url.contains("://"))
    {
        return URL(url);
    }

    if (!url.starts_with("/"))
    {
        auto dir = split(m_urlComponents.path, "/")[0];
        while (url.starts_with("../"))
        {
            url = split(url, "/")[1];
            if (dir.contains("/"))
            {
                dir = split(dir, "/")[0];
            }
        }
        url = dir + "/" + url;
    }

    if (url.starts_with("//"))
    {
        return URL(m_urlComponents.scheme + ":" + url);
    }
    else 
    {
        std::stringstream ss;
        ss << m_urlComponents.scheme;
        ss << "://";
        ss << m_urlComponents.host;
        ss << ":";
        ss << m_urlComponents.port;
        ss << url;
        return URL(ss.str());
    }
}

bool URL::operator==(const URL& other) const
{
    return m_urlComponents == other.m_urlComponents;
}

std::string URL::describe(bool showPort) const
{
    std::stringstream ss;
    ss << m_urlComponents.scheme;
    ss << "://";
    ss << m_urlComponents.host;
    if (showPort)
    {
        ss << ":";
        ss << m_urlComponents.port;
    }
    ss << m_urlComponents.path;
    return ss.str();
}

std::string URL::origin() const
{
    std::stringstream ss;
    ss << m_urlComponents.scheme;
    ss << "://";
    ss << m_urlComponents.host;
    ss << ":";
    ss << m_urlComponents.port;
    return ss.str();
}
