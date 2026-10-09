#include "url_components.h"

#include "helpers/string_helpers.h"
#include <string_view>

UrlComponents getUrlComponents(std::string url)
{
    UrlComponents components;

    // Handle parsing data URLs first.
    if (url.starts_with("data"))
    {
        std::string_view s = url;

        components.scheme = "data";
        components.port = 80;
        s.remove_prefix(5);

        auto comma = s.find(',');
        if (comma == std::string_view::npos)
            return components;

        auto metadata = s.substr(0, comma);
        auto payload = s.substr(comma + 1);

        if (metadata.ends_with(";base64"))
        {
            components.base64 = true;
            metadata.remove_suffix(7);
        }

        components.mediaType = metadata.empty() ? "text/plain;charset=US-ASCII" : std::string(metadata);
        components.data = payload;

        return components;
    }

    // Otherwise, parse the URL as a normal URL.
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
