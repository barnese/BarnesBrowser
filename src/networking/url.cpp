#include "url.h"

#include "helpers/string_helpers.h"

#include <sstream>

URL::URL(const std::string& url)
{
    if (!url.empty())
    {
        m_urlComponents = getUrlComponents(url);
    }
}

URL URL::resolve(std::string url) const
{
    if (url.contains("://"))
    {
        return URL(url);
    }

    if (!url.starts_with("/"))
    {
        auto dir = splitLast(m_urlComponents.path, "/")[0];
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

    if (m_urlComponents.scheme == "data")
    {
        ss << m_urlComponents.scheme << ":";

        if (m_urlComponents.mediaType)
            ss << *m_urlComponents.mediaType;

        if (m_urlComponents.base64)
            ss << ";base64";

        ss << ",";

        if (m_urlComponents.data)
            ss << *m_urlComponents.data;

        return ss.str();
    }
    else if (m_urlComponents.viewSource)
    {
        ss << "view-source:";
    }

    ss << m_urlComponents.scheme;
    ss << "://";
    ss << m_urlComponents.host;
    if (showPort && m_urlComponents.scheme != "file" && m_urlComponents.scheme != "https")
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
