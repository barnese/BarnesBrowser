#pragma once

#include <map>
#include <string>
#include <QGraphicsScene>
#include "point.h"

/// Handles rendering the graphics for the browser.
class BrowserScene: public QGraphicsScene
{
    public:
        BrowserScene();
        void load(std::string url);
        void scrollDown();
        void scrollUp();

    protected:
        void draw();
        void layout(std::string content);
        std::string lex(std::string body);

    private:
        std::map<Point, QChar> m_displayList;
        int m_scroll;
};
