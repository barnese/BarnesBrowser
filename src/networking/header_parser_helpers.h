#pragma once

#include "cache_policy.h"

#include <optional>
#include <string>

/// Parses the status code from the given header text.
std::string parseStatusCode(std::string headerText);

/// Parses the Cache-Control header.
std::optional<CachePolicy> parseCacheControl(std::string headerText);
