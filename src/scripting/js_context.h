#pragma once

#include "duk_config.h"

#include <string>

class JSContext
{
public:
    JSContext();
    ~JSContext();
    std::string run(std::string code);

private:
    duk_context* m_duk;
};
