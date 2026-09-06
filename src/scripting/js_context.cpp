#include "js_context.h"

#include "browser_tab.h"
#include "html_tree/html_parser.h"
#include "html_tree/node_helpers.h"
#include "js_context_exception.h"
#include "styles/css_parser.h"
#include "browser.h"

#include "duktape.h"
#include <dukglue/dukglue.h>
#include <fstream>
#include <iostream>
#include <QThreadPool>

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

JSContext::JSContext(BrowserTab* tab)
    : m_tab(tab)
{
    discarded = false;

    m_duk = duk_create_heap_default();

    /// Exports C++ functions to Duktape's global JavaScript scope.
    dukglue_register_function(m_duk, printMessage, "printMessage");

    dukglue_register_method(m_duk, &JSContext::querySelectorAll, "querySelectorAll");
    dukglue_register_method(m_duk, &JSContext::getAttribute, "getAttribute");
    dukglue_register_method(m_duk, &JSContext::setAttribute, "setAttribute");
    dukglue_register_method(m_duk, &JSContext::setInnerHtml, "setInnerHtml");
    dukglue_register_method(m_duk, &JSContext::setStyle, "setStyle");
    dukglue_register_method(m_duk, &JSContext::sendXmlHttpRequest, "sendXmlHttpRequest");
    dukglue_register_method(m_duk, &JSContext::setTimeout, "setTimeout");
    dukglue_register_method(m_duk, &JSContext::requestAnimationFrame, "requestAnimationFrame");

    // Export the instance of this class to allow the above methods to work.
    dukglue_register_global(m_duk, this, "currentJSContext");

    auto jsRuntime = getJavaScriptRuntime();

    m_tab->browser->measure.start("script-runtime");
    duk_peval_string(m_duk, jsRuntime.c_str());
    m_tab->browser->measure.stop("script-runtime");
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
    auto nodeList = treeToList<Node>(m_tab->getNode());
    
    for (auto& node : nodeList) 
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

void JSContext::setAttribute(int handle, std::string attrName, std::string value)
{
    auto node = getNode(handle);
    if (auto element = dynamic_cast<Element*>(node)) 
    {
        element->attributes[attrName] = value;
        m_tab->setNeedsRender();
    }
}

void JSContext::setInnerHtml(int handle, std::string html)
{
    // Note: hack to get an HTML fragment to work in our parser.
    // Ideally we wouldn't need to wrap the fragment like this.
    html = "<html><body>" + html + "</body></html>";

    auto doc = HTMLParser(html).parse();
    auto& newNodes = doc->children[0]->children;
    auto node = getNode(handle);
    node->children = std::move(newNodes);
    for (auto& child : node->children)
    {
        child->parent = node;
    }
    m_tab->setNeedsRender();
}

void JSContext::setStyle(int handle, std::string style)
{
    auto node = getNode(handle);
    if (auto element = dynamic_cast<Element*>(node))
    {
        element->attributes["style"] = style;
        m_tab->setNeedsRender();
    }    
}

std::string JSContext::runLoad(int handle, URL tabUrl, URL fullUrl, std::string body)
{
    // Make the URL request.
    std::optional<std::string> payload;
    if (!body.empty())
    {
        payload = body;
    }
    auto response = fullUrl.request(tabUrl, payload).bodyString();

    // Schedule a task to send the response back to JavaScript.
    auto task = std::make_unique<Task<JSContext*, int, std::string>>(
        "JSContext::dispatchXhrOnload", 
        &JSContext::dispatchXhrOnload, 
        this,
        handle,
        response
    );
    m_tab->taskRunner.scheduleTask(std::move(task));

    return response;
}

void JSContext::dispatchXhrOnload(int handle, std::string response)
{
    if (discarded) return;
    m_tab->browser->measure.start("script-xhr");
    duk_push_object(m_duk);
    
    duk_push_int(m_duk, handle);
    duk_put_prop_string(m_duk, -2, "handle");
    
    duk_push_string(m_duk, response.c_str());
    duk_put_prop_string(m_duk, -2, "body");
    
    duk_put_global_string(m_duk, "js");
    
    duk_peval_string(m_duk, "__runXhrOnload(js.handle, js.body)");

    duk_pop(m_duk);
    m_tab->browser->measure.stop("script-xhr");
}

std::string JSContext::sendXmlHttpRequest(int handle, std::string method, std::string url, std::string body, bool isAsync)
{
    (void)method; // Silence unused parameter warning.

    auto tabUrl = m_tab->getLoadedUrl();
    auto fullUrl = tabUrl.resolve(url);

    // Make sure this URL is allowed by the Content Security Policy.
    // If there are no allowed origins defined, this allow the style.
    if (!m_tab->allowRequest(fullUrl))
    {
        throw JSContextException("Cross-origin XHR blocked by CSP");
    }

    if (fullUrl.origin() != tabUrl)
    {
        throw JSContextException("Cross-origin XHR not allowed");
    }

    if (!isAsync)
    {
        return runLoad(handle, tabUrl, fullUrl, body);
    }
    else 
    {
        QThreadPool::globalInstance()->start([this, handle, tabUrl, fullUrl, body] {
            runLoad(handle, tabUrl, fullUrl, body);
        });

        return "";
    }
}

void JSContext::dispatchSetTimeout(int handle)
{
    if (discarded) return;
    m_tab->browser->measure.start("script-settimeout");
    duk_push_object(m_duk);
    duk_push_int(m_duk, handle);
    duk_put_prop_string(m_duk, -2, "handle");
    duk_put_global_string(m_duk, "jsContext");
    duk_peval_string(m_duk, "__runSetTimeout(jsContext.handle)");
    duk_pop(m_duk);
    m_tab->browser->measure.stop("script-settimeout");
}

void JSContext::setTimeout(int handle, long time)
{
    QTimer::singleShot(time, this, [this, handle]() {
        auto task = std::make_unique<Task<JSContext*, int>>(
            "JSContext::dispatchSetTimeout", 
            &JSContext::dispatchSetTimeout, 
            this,
            handle
        );
        m_tab->taskRunner.scheduleTask(std::move(task));
    });
}

int JSContext::getHandle(Node* node)
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

Node* JSContext::getNode(int handle)
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
    m_tab->browser->measure.start("script-load");
    auto result = duk_peval_string(m_duk, code.c_str());
    if (result != 0)
    {
        printf("[JSContext] '%s' crashed: %s\n", 
            scriptName.c_str(), 
            duk_safe_to_string(m_duk, -1));
    }
    duk_pop(m_duk);
    m_tab->browser->measure.stop("script-load");
    return result;
}

/// @brief Injects event data into a global JavaScript object 
/// to dispatch that event so event listeners can be executed.
/// @param eventType type of event, such as "click" or "keydown".
/// @param node the node related to this event, such as a button element.
/// @returns int indicating whether the event should be cancelled.
int JSContext::dispatchEvent(std::string eventType, Node* node)
{
    auto handle = getHandle(node);
    
    duk_push_object(m_duk);

    duk_push_string(m_duk, eventType.c_str());
    duk_put_prop_string(m_duk, -2, "type");

    duk_push_int(m_duk, handle);
    duk_put_prop_string(m_duk, -2, "handle");

    duk_put_global_string(m_duk, "jsContext");

    auto doDefault = duk_peval_string(m_duk,
         "new Node(jsContext.handle).dispatchEvent(new Event(jsContext.type))");

    duk_pop(m_duk);

    return !doDefault;
}

void JSContext::requestAnimationFrame()
{
    m_tab->browser->setNeedsAnimationFrame(m_tab);
}

void JSContext::dispatchRunRAFHandlers()
{
    duk_push_object(m_duk);
    duk_peval_string(m_duk, "__runRAFHandlers()");
    duk_pop(m_duk);
}
