#include "js_context.h"

#include "browser_tab.h"
#include "styles/css_parser.h"

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
    std::ifstream file("assets/scripts/runtime.js");
    if (file.is_open())
    {
        return 
        {
            std::istreambuf_iterator<char>(file),       
            std::istreambuf_iterator<char>()
        };
    }
    std::cerr << "Error opening runtime.js!";
    return "";
}

JSContext::JSContext(const BrowserTab* tab)
    : m_tab(tab)
{
    m_duk = duk_create_heap_default();

    /// Exports C++ functions to Duktape's global JavaScript scope.
    dukglue_register_function(m_duk, printMessage, "printMessage");

    dukglue_register_method(m_duk, &JSContext::querySelectorAll, "querySelectorAll");
    dukglue_register_method(m_duk, &JSContext::getAttribute, "getAttribute");

    // For the above "methods" to work, we need an instance of 
    // this class exported to JavaScript, where a wrapper function 
    // can be created in runtime.js to encapsulate access to this object.
    dukglue_register_global(m_duk, this, "currentJSContext");

    auto jsRuntime = getJavaScriptRuntime();
    duk_peval_string(m_duk, jsRuntime.c_str());
}

JSContext::~JSContext()
{
    if (m_duk) 
    {
        duk_destroy_heap(m_duk);
    }
}

std::vector<int> JSContext::querySelectorAll(std::string selectorText)
{
    std::vector<int> nodeHandles;
    auto selector = CSSParser(selectorText).selector();
    auto nodeList = m_tab->nodeTreeToList();
    
    for (const auto& node : nodeList) 
    {    
        if (selector->matches(node))
        {
            nodeHandles.push_back(getHandle(node));
        }
    }

    return nodeHandles;
}

std::string JSContext::getAttribute(int handle, std::string attrName)
{
    auto node = getNode(handle);
    if (const auto& element = dynamic_cast<const Element*>(node)) 
    {
        auto attr = element->findAttribute(attrName, "");
        return attr;
    }
    return "";
}

int JSContext::getHandle(const Node* node)
{
    int handle;
    if (!m_nodeToHandle.contains(node))
    {
        handle = m_nodeToHandle.size();
        m_nodeToHandle[node] = handle;
        m_handleToNode[handle] = node;
    }
    else 
    {
        handle = m_nodeToHandle[node];
    }

    return handle;
}

const Node* JSContext::getNode(int handle)
{
    auto it = m_handleToNode.find(handle);
    return (it != m_handleToNode.end()) ? it->second : nullptr;
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
