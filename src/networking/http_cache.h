#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstddef>

struct CachedResponse
{
    std::vector<std::byte> body;
    std::string contentType;
    std::chrono::steady_clock::time_point expiresAt;
};

/// A simple in-memory cache for HTTP resources.
class HttpCache
{
public:
    const CachedResponse* get(const std::string& url);
    void put(const std::string& url, CachedResponse response);

private:
    std::unordered_map<std::string, CachedResponse> m_cache;
};
