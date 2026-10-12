#pragma once

#include "helpers/case_insensitive_less.h"

#include <map>
#include <string>
#include <vector>

struct URLResponse
{
    std::map<std::string, std::string, CaseInsensitiveLess> headers;
    std::string statusCode;
    std::optional<std::string> errorMessage;
    std::vector<std::byte> body;
    bool viewSource;

    std::string bodyString()
    {
        return std::string(reinterpret_cast<const char*>(body.data()), body.size());
    }
};
