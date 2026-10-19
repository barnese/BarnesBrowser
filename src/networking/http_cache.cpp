#include "http_cache.h"

const CachedResponse* HttpCache::get(const std::string& url)
{
    auto it = m_cache.find(url);

    if (it == m_cache.end())
    {
        return nullptr;
    }

    if (std::chrono::steady_clock::now() >= it->second.expiresAt)
    {
        m_cache.erase(it);
        return nullptr;
    }

    return &it->second;
}

void HttpCache::put(const std::string& url, CachedResponse response)
{
    m_cache[url] = std::move(response);
}
