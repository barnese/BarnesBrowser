#pragma once

#include "commands/command.h"
#include "styles/css_parser.h"
#include "helpers/case_insensitive_less.h"
#include "html_tree/element.h"
#include "html_tree/node.h"
#include "layout/document_layout.h"
#include "tasks/task_runner.h"
#include "point.h"
#include "url.h"

#include <string>
#include <optional>
#include <QImage>
#include <QObject>
#include <QTimer>

class JSContext;

enum BrowserTabStatus
{
    LOADING,
    LOADED,
    ERROR,
};

/// Handles rendering the graphics for a browser tab.
class BrowserTab : public QObject
{
    Q_OBJECT

public:
    BrowserTab(int width, int height);
    ~BrowserTab();

    // Loading and Navigation
    void load(std::string url);
    void load(URL url, std::optional<std::string> payload=std::nullopt);
    std::optional<URL> goBack();
    void didChangeState(bool isActive);
    // Normally, you don't call render directly. Use setNeedsRender() instead.
    void render();
    
    // Event handlers
    void click(Point point);
    void scrollDown();
    void scrollUp();
    void keyPress(std::string text);
    void resize(int width, int height);

    // Public Getters/Setters
    URL getLoadedUrl() { return m_url; };
    const Node* getNode() const { return m_node.get(); };
    QImage* getSurface() { return &m_visibleSurface; };
    void setNeedsRender() { m_needsRender = true; };

    // Public Helpers
    std::vector<const Node*> nodeTreeToList() const;
    std::vector<Node*> nodeTreeToList();

    /// Checks to see if the url is allowed by the Content Security Policy.
    /// If no allowed origins are defined, then the URL is allowed.
    bool allowRequest(const URL& url);

    void scheduleTask(std::unique_ptr<ITask> task);

signals:
    void linkClicked(URL url);

    /// Signal sent when the surface has changed and the
    /// display needs to be updated, namely after a draw.
    void needsDisplayUpdate(const QImage& surface);

private slots:
    void processNextTask();

private:
    /// Copies the viewport of the document based on the
    /// current scroll value and saves it to the visible 
    /// surface. Then sends a display update signal.
    void draw();

    /// Rasters the entire tab's content onto the document surface.
    void raster();

    void loadScripts();
    void loadStyles();
    void loadContentSecurityPolicy(std::map<std::string, std::string, CaseInsensitiveLess> headers);

    void submitForm(Element* element);
    std::vector<std::string> getStyleSheetLinks(const Node* tree);
    std::vector<std::string> getScriptLinks(const Node* tree);
    std::vector<const Node*> nodeTreeToList(const Node* tree, std::vector<const Node*>& list) const;
    std::vector<Node*> nodeTreeToList(Node* tree, std::vector<Node*>& list);

    enum BrowserTabStatus m_status;
    std::unique_ptr<Node> m_node;
    std::vector<std::unique_ptr<Command>> m_displayCommands;
    std::unique_ptr<DocumentLayout> m_document;
    std::unique_ptr<JSContext> m_js;
    URL m_url;
    std::vector<URL> m_history;
    Element* m_focus;
    CSSRules m_rules;
    std::vector<URL> m_allowedOrigins;

    int m_scroll;
    int m_width;
    int m_height;
    
    /// Rasterized backing store for the entire document. (AKA "tab surface")
    QImage m_documentSurface;

    /// Viewport-sized raster buffer for the visible region. (AKA "root surface")
    QImage m_visibleSurface; 

    TaskRunner m_taskRunner;
    QTimer m_timer;

    bool m_isActive;
    
    /// When true, some data in the tab has changed and needs to be re-rendered.
    bool m_needsRender;
};
