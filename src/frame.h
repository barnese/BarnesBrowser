#pragma once

#include "html_tree/node.h"
#include "layout/document_layout.h"
#include "scripting/js_context.h"
#include "styles/css_parser.h"
#include "tasks/task_runner.h"
#include "logger.h"
#include "point.h"
#include "url.h"

#include <optional>
#include <string>

class BrowserTab;
class Element;

/// Frames own the HTML tree, layout tree, scroll offset and events
/// for its HTML document. It runs style and layout on its DOM.
/// It can also construct child frames (iframe).
class Frame
{
public:
    Frame(BrowserTab* tab, Frame* parentFrame, Element* frameElement);

    void load(URL url, std::optional<std::string> payload=std::nullopt);
    void render();
    void setNeedsRender();
    void setNeedsLayout();
    void setFrameSize(double width, double height);

    /// Checks to see if the url is allowed by the Content Security Policy.
    /// If no allowed origins are defined, then the URL is allowed.
    bool allowRequest(const URL& url);

    void dispatchRAF(int windowId);
    void advanceTab();
    void keyPress(std::string text);
    void click(Point point);
    void activateElement(Element* element);

    void scrollTo(Element* element);
    bool isLoaded() { return m_loaded; }
    bool needsStyle() { return m_needsStyle; }
    bool needsLayout() { return m_needsLayout; }
    Node* getNode() { return m_node.get(); }
    Frame* getTabRootFrame();

    /// Prevents scroll from going past top or bottom of the page.
    int clampScroll(const int scroll);
    
    /// Public properties
    TaskRunner taskRunner;
    std::unique_ptr<DocumentLayout> document;
    bool needsFocusScroll;
    bool scrollChangedInFrame;
    int scroll;

private:
    void loadContentSecurityPolicy(std::map<std::string, std::string, CaseInsensitiveLess> headers);
    void parseHtml(const std::string& body);
    void loadScripts();
    void loadStyles();
    void loadImages();
    void loadIFrames();
    void addStyles(Node &node, const CSSRules& rules);
    CSSRules getDefaultStyleSheet();
    std::vector<std::string> getStyleSheetLinks(Node* tree);
    std::vector<std::string> getScriptLinks(Node* tree);
    std::string getTitle(Node* tree);
    void focusElement(Element* element);

    /// Finds all the input elements, encodes their values 
    /// and submits them in a POST request.
    void submitForm(Element* element);

    BrowserTab* m_tab;
    Frame* m_parentFrame;
    Element* m_frameElement;
    std::unique_ptr<Node> m_node;
    std::vector<URL> m_allowedOrigins;
    URL m_url;
    CSSRules m_rules;
    std::unique_ptr<JSContext> m_js;
    bool m_loaded;
    int m_windowId;
    
    /// Dirty flags to indicate when something needs to be re-rendered.
    bool m_needsStyle;
    bool m_needsLayout;
    bool m_needsPaint;
    
    // Size of this frame
    double m_width;
    double m_height;

    Logger m_logger;
};
