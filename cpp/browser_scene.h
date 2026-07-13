#pragma once

#include <string>
#include <QGraphicsScene>
#include "layout/layout.h"

/// Handles rendering the graphics for the browser.
class BrowserScene: public QGraphicsScene
{
public:
    BrowserScene();
    void load(std::string url);
    void scrollDown();
    void scrollUp();

private:
    void draw();
    DisplayList m_displayList;
    int m_scroll;
};
