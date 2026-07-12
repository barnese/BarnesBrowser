#pragma once

#include <map>
#include <string>
#include <QGraphicsScene>
#include "point.h"

// Aliases for convenience
using DisplayList = std::map<Point, std::tuple<QString, QFont>>;

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
