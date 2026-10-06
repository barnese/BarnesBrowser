#include "url_components.h"

#include "helpers/string_helpers.h"

UrlComponents getUrlComponents(std::string url)
{
    UrlComponents components;
    auto parts = split(url, "://");
    components.scheme = parts[0];
    
    auto urlParts = split(parts[1], "/");
    components.host = urlParts[0];
    components.path = "/";
    if (urlParts.size() > 1)
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
