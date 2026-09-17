#pragma once

#include "duk_config.h"
#include "html_tree/node.h"
#include "url.h"

#include <string>
#include <QObject>

class BrowserTab;
class Frame;

/// Provides a context to run JavaScript code.
/// Note: this object is a QObject to get timer functionality.
class JSContext : public QObject
{
public:
    JSContext(BrowserTab* tab, std::string urlOrigin);
    ~JSContext();
    void addWindow(Frame* frame);
    std::string wrap(std::string script, int windowId);
    void throwIfCrossOrigin(Frame* frame);
    std::optional<int> parent(int windowId);
    
    /// @brief Runs the given JavaScript code.
    /// @param scriptName the filename of the script
    /// @param code the JavaScript code to be executed
    /// @param window the window to run the code on
    /// @return 0 if successful, non-zero otherwise
    int run(std::string scriptName, std::string code, int windowId);

    void dispatchRAF(int windowId);
    int dispatchEvent(std::string eventType, Node* node, int windowId);
    
    // Functions to export to JavaScript.
    std::vector<int> querySelectorAll(std::string selectorText, int windowId);
    std::string getAttribute(int handle, std::string attrName, int windowId);
    void setAttribute(int handle, std::string attrName, std::string value, int windowId);
    void setInnerHtml(int handle, std::string html, int windowId);
    void setStyle(int handle, std::string style, int windowId);
    std::string sendXmlHttpRequest(int handle, std::string method, std::string url, std::string body, bool isAsync, int windowId);
    std::string runLoad(int handle, URL frameURL, URL fullUrl, std::string body, int windowId);
    void dispatchXhrOnload(int handle, std::string response, int windowId);
    void dispatchSetTimeout(int handle, int windowId);
    void setTimeout(int handle, long time, int windowId);
    void requestAnimationFrame();
    void postMessage(int targetWindowId, std::string message);
    void dispatchPostMessage(std::string message, int windowId);

    // Flag to discard setTimeout callbacks from running.
    bool discarded;

private:
    /// A handle is a numeric identifier for a node, which is used
    /// as a layer of indirection when sending nodes to JavaScript.
    int getHandle(Node* node);
    Node* getNode(int handle);
    void showEvalError(std::string message);

    /// Maps to correlate handles to nodes and vice versa.
    std::map<Node*, int> m_nodeToHandle;
    std::map<int, Node*> m_handleToNode;

    duk_context* m_duk;
    BrowserTab* m_tab;
    std::string m_urlOrigin;
};
