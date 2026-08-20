#pragma once

#include "duk_config.h"
#include "html_tree/node.h"
#include "url.h"

#include <string>
#include <QObject>

class BrowserTab;

/// Provides a context to run JavaScript code.
/// Note: this object is a QObject to get timer functionality.
class JSContext : public QObject
{
public:
    JSContext(BrowserTab* tab);
    ~JSContext();
    int run(std::string scriptName, std::string code);
    int dispatchEvent(std::string eventType, Node* node);
    
    // Functions to export to JavaScript.
    std::vector<int> querySelectorAll(std::string selectorText);
    std::string getAttribute(int handle, std::string attrName);
    void setInnerHtml(int handle, std::string html);
    void setStyle(int handle, std::string style);
    std::string sendXmlHttpRequest(int handle, std::string method, std::string url, std::string body, bool isAsync);
    std::string runLoad(int handle, URL tabUrl, URL fullUrl, std::string body);
    void dispatchXhrOnload(int handle, std::string response);
    void dispatchSetTimeout(int handle);
    void setTimeout(int handle, long time);
    void requestAnimationFrame();
    void dispatchRunRAFHandlers();

    // Flag to discard setTimeout callbacks from running.
    bool discarded;

private:
    /// A handle is a numeric identifier for a node, which is used
    /// as a layer of indirection when sending nodes to JavaScript.
    int getHandle(Node* node);
    Node* getNode(int handle);

    /// Maps to correlate handles to nodes and vice versa.
    std::map<Node*, int> m_nodeToHandle;
    std::map<int, Node*> m_handleToNode;

    duk_context* m_duk;
    BrowserTab* m_tab;
};
