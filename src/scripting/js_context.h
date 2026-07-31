#pragma once

#include "duk_config.h"
#include "html_tree/node.h"

#include <string>

class BrowserTab;

class JSContext
{
public:
    JSContext(const BrowserTab* tab);
    ~JSContext();
    int run(std::string scriptName, std::string code);
    
    // Functions to export to JavaScript.
    std::vector<int> querySelectorAll(std::string selectorText);
    std::string getAttribute(int handle, std::string attrName);

private:
    /// A handle is a numeric identifier for a node,
    /// which is used as a layer of indirection when 
    /// sending nodes to JavaScript.
    int getHandle(const Node* node);
    const Node* getNode(int handle);

    /// Maps to correlate handles to nodes and vice versa.
    std::map<const Node*, int> m_nodeToHandle;
    std::map<int, const Node*> m_handleToNode;

    duk_context* m_duk;
    const BrowserTab* m_tab;
};
