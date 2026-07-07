#include "browser_scene.h"
#include "constants.h"
#include "url.h"
#include <iostream>
#include <QGraphicsTextItem>

BrowserScene::BrowserScene()
{
    m_scroll = 0;
}

/// Draws the display list to the screen.
/// Draw operates using screen coordinates, which are the
/// visible coordinates on the screen based on the current
/// scroll value.
void BrowserScene::draw()
{
    // Erase the entire scene so we can redraw its contents.
    clear();

    std::map<Point, QChar>::iterator it;
    for (it = m_displayList.begin(); it != m_displayList.end(); ++it)
    {
        Point point = it->first;
        QChar c = it->second;
        addText(c)->setPos(point.x, point.y - m_scroll);
    }
}

/// Updates the layout's display list for the given display content.
/// Layout operates using page coordinates, which represent 
/// an entire page, not only what is visible on the screen.
void BrowserScene::layout(std::string content)
{
    m_displayList.clear();

    int cursorX = Constants::HSTEP;
    int cursorY = Constants::VSTEP;

    QString text = QString::fromUtf8(content.data(), content.size());

    for (QChar c : text)
    {
        Point point(cursorX, cursorY);
        m_displayList[point] = c;

        cursorX += Constants::HSTEP;

        if (cursorX >= Constants::WIDTH - Constants::HSTEP)
        {
            cursorY += Constants::VSTEP;
            cursorX = Constants::HSTEP;
        }
    }
}

/// Gets the textual content of an HTML document.
std::string BrowserScene::lex(std::string body)
{
    std::string text;
    bool inTag = false;

    for (char c : body) 
    {
        if (c == '<')
            inTag = true;
        else if (c == '>')
            inTag = false;
        else if (!inTag) 
            text += c;
    }

    return text;
}

/// Loads the HTML page from the given URL.
void BrowserScene::load(std::string urlString)
{

    try
    {
        URL url(urlString);
        auto body = url.request();
        auto content = lex(body);
        layout(content);
        draw();
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << "Error: " << error.what() << "\n";
    }
}

void BrowserScene::scrollDown()
{
    m_scroll += Constants::SCROLL_STEP;
    draw();
}

void BrowserScene::scrollUp()
{
    m_scroll -= Constants::SCROLL_STEP;
    if (m_scroll < 0) 
    {
        m_scroll = 0;
    }
    draw();
}
