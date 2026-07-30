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

/// @brief Runs the given JavaScript code.
/// @param scriptName the filename of the script
/// @param code the JavaScript code to be executed
/// @return 0 if successful, non-zero otherwise
int JSContext::run(std::string scriptName, std::string code)
{
    auto result = duk_peval_string(m_duk, code.c_str());
    if (result != 0)
    {
        printf("[JSContext] '%s' crashed: %s\n", 
            scriptName.c_str(), 
            duk_safe_to_string(m_duk, -1));
    }
    duk_pop(m_duk);
    return result;
}
