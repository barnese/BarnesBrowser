#include "url.h"
#include "helpers.h"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

URL::URL(const std::string& url)
{
    auto parts = split(url, "://");
    m_scheme = parts[0];
    
    auto urlParts = split(parts[1], "/");
    m_host = urlParts[0];
    m_path = "/" + urlParts[1];
    m_port = 80;
}

void URL::request() const
{
    // Do a DNS lookup to resolve the host using TCP.
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* result = nullptr;
    std::string port = std::to_string(m_port);
    int address = getaddrinfo(m_host.c_str(), port.c_str(), &hints, &result);

    if (address != 0)
    {
        std::cout << "DNS lookup failed! Error code = " << address << "\n";
        return;
    }

    // Create the socket.
    int sock = socket(result->ai_family, 
                    result->ai_socktype,
                    result->ai_protocol);

    if (sock == 0)
    {
        std::cout << "Failed to create the socket!\n";
        return;
    }

    // Connect to the host.
    if (connect(sock, result->ai_addr, result->ai_addrlen) == -1) 
    {
        std::cout << "Error connecting to the host!\n";
        return;
    }

    // Create the HTTP request.
    std::ostringstream out;
    out << "GET " << m_path << " HTTP/1.0\r\n";
    out << "Host: " << m_host << "\r\n";
    out << "\r\n";
    std::string request = out.str();

    // Send the request.
    size_t totalSent = 0;
    while (totalSent < request.size())
    {
        ssize_t sent = send(
            sock,
            request.data() + totalSent,
            request.size() - totalSent,
            0
        );

        if (sent <= 0)
        {
            std::cout << "there was an error sending the request!\n";
            break;
        }

        totalSent += sent;
    }

    // Receive the response.
    std::string response;
    char buffer[4096];
    while (true)
    {
        ssize_t received = recv(sock, buffer, sizeof(buffer), 0);

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

    close(sock);

    // Parse the response.
    const std::string separator = "\r\n\r\n";
    size_t headerEnd = response.find(separator);

    std::string headerText = response.substr(0, headerEnd);
    std::string body = response.substr(headerEnd + separator.size());

    std::istringstream stream(headerText);
    std::string statusLine;
    std::getline(stream, statusLine);
    std::istringstream statusStream(statusLine);
    std::string version;
    std::string statusCode;
    statusStream >> version;
    statusStream >> statusCode;

    std::cout << "Received response:\n";
    std::cout << "Version = " << version << "\n";
    std::cout << "Status Code = " << statusCode << "\n";
    
    // TODO: Finish parsing the response headers.
}

void URL::describe() const
{
    std::cout << "scheme = " << m_scheme << "\n";
    std::cout << "host = " << m_host << "\n";
    std::cout << "path = " << m_path << "\n";
    std::cout << "port = " << m_port << "\n";
}
