#include "resource_loader.h"

#include "helpers/file_helpers.h"
#include "helpers/string_helpers.h"
#include "globals.h"
#include "header_parser_helpers.h"

#include <cassert>
#include <iostream>
#include <optional>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

ResourceLoader::ResourceLoader(bool keepAlive)
    : m_keepAlive(keepAlive)
{
    m_connection = std::make_unique<HttpConnection>();
}

ResourceLoader::~ResourceLoader()
{
    m_connection->close();
}

URLResponse ResourceLoader::request(
    URL url, 
    std::optional<URL> referrer, 
    std::optional<std::string> payload)
{
    auto components = url.components();

    if (components.aboutBlank)
    {
        return aboutBlank();
    }
    else if (components.scheme == "file")
    {
        return requestFromFile(components.path, components.viewSource);
    }
    else if (components.scheme == "data")
    {
        return requestFromData(components);
    }
    else 
    {
        return requestFromNetwork(url, referrer, payload);
    }
}

void addCookieToRequest(std::string host, std::string method, const std::optional<URL>& referrer, std::ostringstream& out)
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

std::string formatGetRequest(
    std::string path, 
    std::string host, 
    const std::optional<URL>& referrer)
{
    std::ostringstream out;
    out << "GET " << path << " HTTP/1.1\r\n";
    out << "Host: " << host << "\r\n";    
    out << "Connection: keep-alive\r\n";
    out << "User-Agent: BBrowser/1.0\r\n";
    addCookieToRequest(host, "GET", referrer, out);
    out << "\r\n";
    return out.str();
}

std::string formatPostRequest(
    std::string path, 
    std::string host, 
    const std::optional<URL>& referrer, 
    std::string payload)
{
    std::ostringstream out;
    out << "POST " << path << " HTTP/1.1\r\n";
    out << "Host: " << host << "\r\n";
    out << "Connection: keep-alive\r\n";
    out << "User-Agent: BBrowser/1.0\r\n";
    out << "Content-Length: " << payload.size() << "\r\n";
    addCookieToRequest(host, "POST", referrer, out);
    out << "\r\n";
    out << payload;
    return out.str();
}

/// Determines if the status code is a redirect.
bool isRedirect(int statusCode)
{
    return statusCode >= 301 && statusCode <= 308;
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

std::string ResourceLoader::readHeaders(std::span<const std::byte> data, size_t& pos) const
{
    const size_t start = pos;

    while (pos + 3 < data.size())
    {
        // If we find the header terminator (\r\n\r\n) return the 
        // headers and advance pos to the beginning of the body.
        if (data[pos] == std::byte{'\r'} && 
            data[pos + 1] == std::byte{'\n'} &&
            data[pos + 2] == std::byte{'\r'} && 
            data[pos + 3] == std::byte{'\n'})
        {
           std::string headers(reinterpret_cast<const char*>(data.data() + start), pos - start); 

           pos += 4;

           return headers;
        }

        ++pos;
    }

    // The header terminator wasn't found.
    return {};
}

URLResponse ResourceLoader::aboutBlank() const
{
    URLResponse response;
    std::string body = "&nbsp;";
    std::vector<std::byte> bodyBytes;
    bodyBytes.reserve(body.size());

    std::transform(body.begin(), body.end(), std::back_inserter(bodyBytes), [](unsigned char c) {
        return static_cast<std::byte>(c);
    });

    response.body = bodyBytes;

    return response;
}

URLResponse ResourceLoader::requestFromFile(const std::string& path, bool viewSource) const
{
    URLResponse response;
    response.body = readFile(path);
    response.viewSource = viewSource;
    return response;
}

URLResponse ResourceLoader::requestFromData(const UrlComponents& components) const
{
    URLResponse response;
    if (components.data)
    {
        auto data = *components.data;

        if (components.base64)
        {
            auto decoded = decodeBase64(data);
            if (decoded)
                data = *decoded;
        }

        std::vector<std::byte> result;
        result.reserve(data.size());
        for (unsigned char c : data)
        {
            result.push_back(static_cast<std::byte>(c));
        }
        response.body = result;
    }
    return response;
}

URLResponse ResourceLoader::requestFromNetwork(URL url, const std::optional<URL>& referrer, std::optional<std::string> payload)
{
    URLResponse response;
    response.viewSource = url.components().viewSource;

    std::string urlString = url.describe();

    // If we have already cached the response for this request, just return it.
    if (auto cached = m_httpCache.get(urlString))
    {
        std::cout << "[ResourceLoader] using cache for " << urlString << std::endl;
        response.body = cached->body;
        return response;
    }

    UrlComponents urlComponents = url.components();

    // Loop to handle URLs that respond with a redirect responses, 
    // but only up to 10 times to avoid infinite redirect loops.
    for (int redirectCount = 0; redirectCount < 10; ++redirectCount)
    {
        std::cout << "[ResourceLoader] requesting " << urlString << std::endl;
        std::string request;
        if (payload == std::nullopt)
        {
            request = formatGetRequest(
                urlComponents.path, 
                urlComponents.host, 
                referrer);
        }
        else
        {
            request = formatPostRequest(
                urlComponents.path, 
                urlComponents.host, 
                referrer, 
                payload.value());
        }

        //debugOutRequest(request);
        m_connection->sendRequest(url, request);

        std::string headerData;
        std::map<std::string, std::string, CaseInsensitiveLess> headers;
        std::vector<std::byte> body;

        if (!m_keepAlive)
        {
            m_connection->receiveResponse();
            headerData = m_connection->readHeaders();
            headers = parseHeaders(headerData);
            body = m_connection->readBody();
            m_connection->close();
        }
        else
        {
            // Using keep-alive, read the headers first, so
            // that we can get the content-length of the body.
            headerData = m_connection->readHeaders();
            headers = parseHeaders(headerData);
           
            if (headers.contains("Transfer-Encoding"))
            {
                throw std::runtime_error("Transfer-Encoding not supported yet");
            }
            
            size_t contentLength = std::stoul(headers["Content-Length"]);

            body = m_connection->readBody(contentLength);

            // Cache the result if we receive a cache control header
            // with a max-age directive from the server.
            if (headers.contains("Cache-Control"))
            {                
                auto cachePolicy = parseCacheControl(headers["Cache-Control"]);

                if (!cachePolicy->noStore && cachePolicy->maxAge)
                {
                    auto contentType = headers["Content-Type"];
                    
                    CachedResponse entry {
                        .body = body,
                        .contentType = contentType,
                        .expiresAt = std::chrono::steady_clock::now() + *cachePolicy->maxAge
                    };

                    m_httpCache.put(urlString, std::move(entry));
                }
            }
        }

        response.statusCode = parseStatusCode(headerData);
        response.headers = headers;
        response.body = body;

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
        auto fullUrl = url.resolve(location);
        urlComponents = fullUrl.components();
        std::cout << "[ResourceLoader] Redirecting to: " << fullUrl.describe(false) << std::endl;
    }

    response.errorMessage = "Too many redirects!";
    return response;
}

void ResourceLoader::debugOutRequest(const std::string& request)
{
#if NETWORK_DEBUG_OUTPUT
    std::cout << "[ResourceLoader] sending request:" << std::endl << request;
#else
    (void)request; // silence warning
#endif
}
