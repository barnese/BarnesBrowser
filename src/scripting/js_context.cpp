#include "js_context.h"

#include "duktape.h"

#include <dukglue/dukglue.h>
#include <fstream>
#include <iostream>

void printMessage(const std::string& message)
{
    std::cout << message << std::endl;
}

std::string getJavaScriptRuntime()
{
    std::string content;
    std::ifstream file("assets/scripts/runtime.js");

    if (file.is_open())
    {
        content = {
            std::istreambuf_iterator<char>(file),       
            std::istreambuf_iterator<char>()
        };
    }
    else 
    {
        std::cerr << "Error opening runtime.js!";
        content = "";
    }

    return content;
}

JSContext::JSContext()
{
    m_duk = duk_create_heap_default();

    /// Exports C++ functions to Duktape's global JavaScript scope.
    dukglue_register_function(m_duk, printMessage, "logMessage");

    auto jsRuntime = getJavaScriptRuntime();
    duk_peval_string(m_duk, jsRuntime.c_str());
}

JSContext::~JSContext()
{
    duk_destroy_heap(m_duk);
}

std::string JSContext::run(std::string code)
{
    duk_peval_string(m_duk, code.c_str());    
    return "";
}
