#pragma once

#include "commands/command.h"
#include "html_tree/node.h"
#include "layout/document_layout.h"
#include "point.h"
#include "url.h"

#include <string>
#include <QGraphicsScene>

/// Handles rendering the graphics for the browser.
class BrowserScene: public QGraphicsScene
{
public:
    BrowserScene();
    void load(std::string url);
    void load(URL url);
    void click(Point point);
    void scrollDown();
    void scrollUp();

private:
    void draw();
    
    std::vector<std::unique_ptr<Command>> m_displayCommands;
    int m_scroll;
    std::unique_ptr<DocumentLayout> m_document;
    URL m_url;
    std::unique_ptr<Node> m_node;
};
