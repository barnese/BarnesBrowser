#pragma once

#include "accessibility/accessibility_node.h"
#include "commands/command.h"
#include "styles/css_parser.h"
#include "helpers/case_insensitive_less.h"
#include "html_tree/element.h"
#include "html_tree/node.h"
#include "layout/document_layout.h"
#include "tasks/task_runner.h"
#include "point.h"
#include "logger.h"
#include "url.h"

#include <string>
#include <optional>
#include <QObject>

class JSContext;
class Browser;

/// Handles rendering the graphics for a browser tab.
class BrowserTab : public QObject
{
    Q_OBJECT

public:
    BrowserTab(int width, int height);
    ~BrowserTab();

    // Loading and Navigation
    void load(URL url, std::optional<std::string> payload=std::nullopt);
    std::optional<URL> goBack();
    void render();
    void setNeedsRender();
    void setNeedsLayout();
    void setNeedsPaint();
    void runAnimationFrame(const int scroll);
    
    // Event handlers
    void click(Point point);
    void keyPress(std::string text);
    void resize(int width, int height);
    void zoom(bool increment);
    void resetZoom();
    void setDarkMode(bool darkMode);
    void advanceTab();
    void enter();

    // Public Getters/Setters
    URL getLoadedUrl() { return m_url; };
    Node* getNode() { return m_node.get(); };
    bool isLoaded() { return m_loaded; };

    /// Checks to see if the url is allowed by the Content Security Policy.
    /// If no allowed origins are defined, then the URL is allowed.
    bool allowRequest(const URL& url);

    /// A pointer back to the main browser object.
    Browser* browser;
    TaskRunner taskRunner;

private:
    /// Prevents scroll from going past top or bottom of the page.
    int clampScroll(const int scroll);

    void addStyles(Node &node, const CSSRules& rules);
    void paintTree(const Layout& layout, std::vector<std::unique_ptr<Command>>& displayList);
    void loadScripts();
    void loadStyles();
    void loadImages();
    void loadContentSecurityPolicy(std::map<std::string, std::string, CaseInsensitiveLess> headers);
    void submitForm(Element* element);
    void activateElement(Element* element);
    void focusElement(Element* element);
    void scrollTo(Element* element);

    CSSRules getDefaultStyleSheet();
    std::vector<std::string> getStyleSheetLinks(Node* tree);
    std::vector<std::string> getScriptLinks(Node* tree);
    std::string getTitle(Node* tree);
    void setTabTitle();

    std::unique_ptr<Node> m_node;
    std::optional<std::vector<std::unique_ptr<Command>>> m_displayList;    
    std::vector<Node*> m_compositedUpdates;
    std::unique_ptr<DocumentLayout> m_document;
    std::unique_ptr<AccessibilityNode> m_accessibilityTree;
    std::unique_ptr<JSContext> m_js;
    URL m_url;
    std::vector<URL> m_history;
    Element* m_focus;
    CSSRules m_rules;
    std::vector<URL> m_allowedOrigins;

    bool m_loaded;
    int m_scroll;
    bool m_scrollChangedInTab;
    int m_width;
    int m_height;
    double m_zoom;
    bool m_darkMode;
    bool m_needsAccessibility;
    
    /// Dirty flags to indicate when something needs to be re-rendered.
    bool m_needsStyle;
    bool m_needsLayout;
    bool m_needsPaint;
    bool m_needsFocusScroll;

    Logger m_logger;
};
