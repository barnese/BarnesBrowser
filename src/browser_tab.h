#pragma once

#include "accessibility/accessibility_node.h"
#include "commands/command.h"
#include "html_tree/element.h"
#include "html_tree/node.h"
#include "networking/url.h"
#include "scripting/js_context.h"
#include "tasks/task_runner.h"
#include "logger.h"
#include "point.h"

#include <string>
#include <optional>
#include <QObject>

class Browser;
class Frame;
class JSContext;

/// Represents a browser tab with these purposes:
/// - interface between the Browser and Frames
/// - proxy communication between frames
/// - run animation frames and rendering 
/// - paint and own the display list for all frames in the tab
/// - construct and own the accessibility tree
/// - commit to the browser thread
class BrowserTab : public QObject
{
    Q_OBJECT

public:
    BrowserTab(int width, int height);
    ~BrowserTab();

    // Loading and Navigation
    void load(URL url, std::optional<std::string> payload=std::nullopt);
    void goBack();
    void render();
    void setNeedsRenderAllFrames();
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
    void scrollDown();
    void postMessage(std::string message, int targetWindowId);

    // Public Getters/Setters
    URL getLoadedUrl() { return m_url; }
    Element* getFocus() { return m_focus; }
    void setFocus(Element* element) { m_focus = element; }
    Frame* getFocusedFrame() { return m_focusedFrame; }
    void setFocusedFrame(Frame* frame) { m_focusedFrame = frame; }
    bool isLoaded() { return m_loaded; }
    void setNeedsAccessibility(bool value) { m_needsAccessibility = value; }
    JSContext* getJs(URL url);
    bool isDarkMode() { return m_darkMode; }
    double getZoom() { return m_zoom; }
    void setTabAddress(const std::string& address, const std::string& title);
    void addHistory(URL url) { m_history.push_back(url); }
    Frame* getRootFrame() { return m_rootFrame.get(); }
    Frame* getFrameByWindowId(int windowID);

    /// Checks to see if the url is allowed by the Content Security Policy.
    /// If no allowed origins are defined, then the URL is allowed.
    bool allowRequest(const URL& url);

    /// A pointer back to the main browser object.
    Browser* browser;
    TaskRunner taskRunner;

    /// Maps window IDs to frames. Here "window" really means "frame".
    /// I'm using a vector here to preserve the order in which entries
    /// are added, since maps sort by key. This is important when
    /// rendering the frames, which must be in order they were added.
    std::vector<std::pair<int, Frame*>> windowIdToFrame;

private:
    /// Recursively paints all layout objects, combining the 
    /// results into the provided display commands reference.
    void paintTree(Layout& layout, std::vector<std::unique_ptr<Command>>& displayList);
    void activateElement(Element* element);

    std::unique_ptr<Frame> m_rootFrame;
    std::optional<std::vector<std::unique_ptr<Command>>> m_displayList;    
    std::vector<Node*> m_compositedUpdates;
    std::unique_ptr<AccessibilityNode> m_accessibilityTree;
    URL m_url;
    std::vector<URL> m_history;
    Element* m_focus = nullptr;
    Frame* m_focusedFrame = nullptr;
    std::vector<URL> m_allowedOrigins;
    std::map<std::string, std::unique_ptr<JSContext>> m_originToJs;

    bool m_loaded;
    int m_scroll;
    bool m_scrollChangedInTab;
    int m_width;
    int m_height;
    double m_zoom;
    bool m_darkMode;
    
    /// Dirty flags to indicate when something needs to be re-rendered.
    bool m_needsPaint;
    bool m_needsAccessibility;

    Logger m_logger;
};
