#pragma once

#include "helpers/case_insensitive_less.h"

#include <map>
#include <string>

namespace Globals
{
    /// Defines a global cookie jar where the key is a host 
    /// and the value is the cookie and its key/value parameters. 
    /// The map is set up to allow case insensitive lookups.
    inline std::map<std::string, // host
        std::tuple<std::string, std::map<std::string, std::string>>, // cookie, map of parameters
        CaseInsensitiveLess> cookieJar = {};
}
