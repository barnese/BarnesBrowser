#pragma once

#include "commands/command.h"
#include "styles/css_parser.h"
#include "html_tree/element.h"
#include "html_tree/node.h"
#include "layout/document_layout.h"
#include "point.h"
#include "url.h"

#include <string>
#include <optional>
#include <QGraphicsScene>

class JSContext;

enum BrowserTabStatus
{
    LOADING,
    LOADED,
    ERROR,
};

/// Handles rendering the graphics for a browser tab.
class BrowserTab: public QGraphicsScene
{
    Q_OBJECT

public:
    BrowserTab(int width, int height);
    ~BrowserTab();
    void load(std::string url);
    void load(URL url, std::optional<std::string> payload=std::nullopt);
    void click(Point point);
    std::optional<URL> goBack();
    void scrollDown();
    void scrollUp();
    void keyPress(std::string text);
    void resize(int width, int height);
    const Node* getNode() const { return m_node.get(); };
    std::vector<const Node*> nodeTreeToList() const;
    std::vector<Node*> nodeTreeToList();
    void render();
    URL getLoadedUrl() { return m_url; };

signals:
    void linkClicked(URL url);

private:
    void draw();
    void loadScripts();
    void loadStyles();
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

    int m_scroll;
    int m_width;
    int m_height;
};
