#include "js_context.h"

#include "browser_tab.h"
#include "html_tree/html_parser.h"
#include "html_tree/node_helpers.h"
#include "js_context_exception.h"
#include "styles/css_parser.h"
#include "browser.h"
#include "frame.h"

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

JSContext::JSContext(BrowserTab* tab, std::string urlOrigin)
    : m_tab(tab),
      m_urlOrigin(urlOrigin)
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

    duk_peval_string(m_duk, "function Window(id) { this._id = id };");
    duk_peval_string(m_duk, "WINDOWS = {}");
}

JSContext::~JSContext()
{
    if (m_duk) 
    {
        duk_destroy_heap(m_duk);
    }
}

void JSContext::addWindow(Frame* frame)
{
    auto id = frame->windowId;
    std::stringstream code;
    code << "var window_" << id << " = new Window(" << id << ");";
    duk_peval_string(m_duk, code.str().c_str());

    auto jsRuntime = getJavaScriptRuntime();
    auto wrappedRuntime = wrap(jsRuntime, id);
    m_tab->browser->measure.start("script-runtime");
    duk_peval_string(m_duk, wrappedRuntime.c_str());
    m_tab->browser->measure.stop("script-runtime");

    std::stringstream mapCode;
    mapCode << "WINDOWS[" << id << "] = window_" << id << ";";
    duk_peval_string(m_duk, mapCode.str().c_str());
}

std::string JSContext::wrap(std::string script, int windowId)
{
    std::stringstream code;
    code << "window = window_" << windowId << "; ";
    code << script;
    return code.str();
}

void JSContext::throwIfCrossOrigin(Frame* frame)
{
    if (frame->getURL().origin() != m_urlOrigin)
    {
        throw JSContextException("Cross-origin access disallowed from script");
    }
}

std::optional<int> JSContext::parent(int windowId)
{
    auto parentFrame = m_tab->getFrameByWindowId(windowId)->getParentFrame();
    if (!parentFrame)
        return std::nullopt;
    return parentFrame->windowId;
}

int JSContext::run(std::string scriptName, std::string code, int windowId)
{
    m_tab->browser->measure.start("script-load");
    code = wrap(code, windowId);
    auto result = duk_peval_string(m_duk, code.c_str());
    if (result != 0)
    {
        printf("[JSContext] '%s' run crashed: %s\n", 
            scriptName.c_str(), 
            duk_safe_to_string(m_duk, -1));
    }
    duk_pop(m_duk);
    m_tab->browser->measure.stop("script-load");
    return result;
}

std::vector<int> JSContext::querySelectorAll(std::string selectorText, int windowId)
{
    auto frame = m_tab->getFrameByWindowId(windowId);
    throwIfCrossOrigin(frame);
    std::vector<int> nodeHandles;
    auto selector = CSSParser(selectorText).selector();
    auto nodeList = treeToList<Node>(frame->getNode());
    
    for (auto& node : nodeList) 
    {    
        if (selector->matches(node))
        {
            nodeHandles.push_back(getHandle(node));
        }
    }

    return nodeHandles;
}

std::string JSContext::getAttribute(int handle, std::string attrName, int windowId)
{
    auto frame = m_tab->getFrameByWindowId(windowId);
    throwIfCrossOrigin(frame);
    auto node = getNode(handle);
    if (const auto& element = dynamic_cast<const Element*>(node)) 
    {
        auto attr = element->findAttribute(attrName, "");
        return attr;
    }
    return "";
}

void JSContext::setAttribute(int handle, std::string attrName, std::string value, int windowId)
{
    auto frame = m_tab->getFrameByWindowId(windowId);
    throwIfCrossOrigin(frame);
    auto node = getNode(handle);
    if (auto element = dynamic_cast<Element*>(node)) 
    {
        element->attributes[attrName] = value;
        m_tab->setNeedsRenderAllFrames();
    }
}

void JSContext::setInnerHtml(int handle, std::string html, int windowId)
{
    auto frame = m_tab->getFrameByWindowId(windowId);
    throwIfCrossOrigin(frame);

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
    frame->setNeedsRender();
}

void JSContext::setStyle(int handle, std::string style, int windowId)
{
    auto frame = m_tab->getFrameByWindowId(windowId);
    throwIfCrossOrigin(frame);
    auto node = getNode(handle);
    if (auto element = dynamic_cast<Element*>(node))
    {
        element->attributes["style"] = style;
        frame->setNeedsRender();
    }    
}

void JSContext::dispatchXhrOnload(int handle, std::string response, int windowId)
{
    if (discarded) return;
    m_tab->browser->measure.start("script-xhr");
    duk_push_object(m_duk);
    
    duk_push_int(m_duk, handle);
    duk_put_prop_string(m_duk, -2, "handle");
    
    duk_push_string(m_duk, response.c_str());
    duk_put_prop_string(m_duk, -2, "body");
    
    duk_put_global_string(m_duk, "js");
    
    std::string code = wrap("window.__runXhrOnload(js.handle, js.body)", windowId);
    auto result = duk_peval_string(m_duk, code.c_str());

    if (result != 0)
        showEvalError("dispatch XHR onload error");

    duk_pop(m_duk);
    m_tab->browser->measure.stop("script-xhr");
}

std::string JSContext::sendXmlHttpRequest(int handle, std::string method, std::string url, std::string body, bool isAsync, int windowId)
{
    (void)method; // Silence unused parameter warning.
    
    auto frame = m_tab->getFrameByWindowId(windowId);

    auto frameUrl = frame->getURL();
    auto fullUrl = frameUrl.resolve(url);

    // Make sure this URL is allowed by the Content Security Policy.
    // If there are no allowed origins defined, this allow the style.
    if (!frame->allowRequest(fullUrl))
    {
        throw JSContextException("Cross-origin XHR blocked by CSP");
    }

    if (fullUrl.origin() != frameUrl.origin())
    {
        throw JSContextException("Cross-origin XHR not allowed");
    }

    if (!isAsync)
    {
        return runLoad(handle, frame->getURL(), fullUrl, body, windowId);
    }
    else 
    {
        QThreadPool::globalInstance()->start([this, handle, frameUrl, fullUrl, body, windowId] {
            runLoad(handle, frameUrl, fullUrl, body, windowId);
        });

        return "";
    }
}

std::string JSContext::runLoad(int handle, URL frameUrl, URL fullUrl, std::string body, int windowId)
{
    // Make the URL request.
    std::optional<std::string> payload;
    if (!body.empty())
    {
        payload = body;
    }
    auto response = fullUrl.request(frameUrl, payload).bodyString();

    // Schedule a task to send the response back to JavaScript.
    auto task = std::make_unique<Task<JSContext*, int, std::string, int>>(
        "JSContext::dispatchXhrOnload", 
        &JSContext::dispatchXhrOnload, 
        this,
        handle,
        response,
        windowId
    );
    m_tab->taskRunner.scheduleTask(std::move(task));

    return response;
}

void JSContext::dispatchSetTimeout(int handle, int windowId)
{
    if (discarded) return;
    m_tab->browser->measure.start("script-settimeout");
    duk_push_object(m_duk);
    duk_push_int(m_duk, handle);
    duk_put_prop_string(m_duk, -2, "handle");
    duk_put_global_string(m_duk, "jsContext");
    auto code = wrap("window.__runSetTimeout(jsContext.handle)", windowId);
    auto result = duk_peval_string(m_duk, code.c_str());
    if (result != 0)
        showEvalError("dispatch set timeout error");
    duk_pop(m_duk);
    m_tab->browser->measure.stop("script-settimeout");
}

void JSContext::setTimeout(int handle, long time, int windowId)
{
    QTimer::singleShot(time, this, [this, handle, windowId]() {
        auto task = std::make_unique<Task<JSContext*, int, int>>(
            "JSContext::dispatchSetTimeout", 
            &JSContext::dispatchSetTimeout, 
            this,
            handle,
            windowId
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

void JSContext::dispatchRAF(int windowId)
{
    duk_push_object(m_duk);
    std::string code = wrap("window.__runRAFHandlers()", windowId);
    auto result = duk_peval_string(m_duk, code.c_str());
    if (result != 0)
        showEvalError("dispatch RAF error");
    duk_pop(m_duk);
}

/// @brief Injects event data into a global JavaScript object 
/// to dispatch that event so event listeners can be executed.
/// @param eventType type of event, such as "click" or "keydown".
/// @param node the node related to this event, such as a button element.
/// @returns int indicating whether the event should be cancelled.
int JSContext::dispatchEvent(std::string eventType, Node* node, int windowId)
{
    auto handle = getHandle(node);
    
    duk_push_object(m_duk);

    duk_push_string(m_duk, eventType.c_str());
    duk_put_prop_string(m_duk, -2, "type");

    duk_push_int(m_duk, handle);
    duk_put_prop_string(m_duk, -2, "handle");

    duk_put_global_string(m_duk, "jsContext");

    std::string code = "new window.Node(jsContext.handle).dispatchEvent(new window.Event(jsContext.type))";
    code = wrap(code, windowId);
    auto result = duk_peval_string(m_duk, code.c_str());

    if (result != 0)
        showEvalError("dispatch event error");

    duk_pop(m_duk);

    return !result;
}

void JSContext::requestAnimationFrame()
{
    m_tab->browser->setNeedsAnimationFrame(m_tab);
}

void JSContext::postMessage(int targetWindowId, std::string message)
{    
    auto task = std::make_unique<Task<BrowserTab*, std::string, int>>(
        "BrowserTab::postMessage(" + message + ")", 
        &BrowserTab::postMessage, 
        m_tab,
        message,
        targetWindowId
    );
    m_tab->taskRunner.scheduleTask(std::move(task)); 
}

void JSContext::dispatchPostMessage(std::string message, int windowId)
{
    duk_push_object(m_duk);
    duk_push_string(m_duk, message.c_str());
    duk_put_prop_string(m_duk, -2, "message");
    duk_put_global_string(m_duk, "jsContext");

    std::string code = "window.dispatchEvent(new window.MessageEvent(jsContext.message))";
    code = wrap(code, windowId);
    auto result = duk_peval_string(m_duk, code.c_str());
    if (result != 0)
        showEvalError("dispatch post message error");
    duk_pop(m_duk);
}

void JSContext::showEvalError(std::string message)
{
    std::cerr << "[JSContext] "
              << message << ": " 
              << duk_safe_to_string(m_duk, -1)
              << std::endl;
}
