#pragma once

#include "commands/command.h"
#include "html_tree/element.h"
#include "html_tree/node.h"
#include "layout/document_layout.h"
#include "point.h"
#include "url.h"

#include <string>
#include <QGraphicsScene>

/// Handles rendering the graphics for a browser tab.
class BrowserTab: public QGraphicsScene
{
    Q_OBJECT

public:
    BrowserTab(int width, int height);
    void load(std::string url);
    void load(URL url);
    void render();
    void click(Point point);
    std::optional<URL> goBack();
    void scrollDown();
    void scrollUp();
    void keyPress(std::string text);
    void resize(int width, int height);

signals:
    void linkClicked(URL url);

private:
    void draw();
    
    std::unique_ptr<Node> m_node;
    std::vector<std::unique_ptr<Command>> m_displayCommands;
    std::unique_ptr<DocumentLayout> m_document;
    URL m_url;
    std::vector<URL> m_history;
    Element* m_focus;

    int m_scroll;
    int m_width;
    int m_height;
};
