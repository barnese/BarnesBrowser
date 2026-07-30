#pragma once

#include "duk_config.h"

#include <string>

class JSContext
{
public:
    JSContext();
    ~JSContext();
    int run(std::string scriptName, std::string code);

private:
    duk_context* m_duk;
};
