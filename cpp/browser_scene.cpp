#include "browser_scene.h"
#include "constants.h"
#include "html_tree/html_parser.h"
#include "layout.h"
#include "url.h"
#include "html_tree/element.h"
#include "html_tree/text.h"
#include <iostream>
#include <vector>
#include <QGraphicsTextItem>
#include <QFont>

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

    DisplayList::iterator it;
    for (it = m_displayList.begin(); it != m_displayList.end(); ++it)
    {
        Point point = it->first;

        // Skip drawing content that is off-screen.
        if (point.y > m_scroll + Constants::HEIGHT || 
            point.y + Constants::VSTEP < m_scroll) 
        {
            continue;
        }

        std::tuple<QString, QFont> item(it->second);
        QString itemText = std::get<0>(item);
        QFont font = std::get<1>(item);
        auto text = addText(itemText, font);
        text->setPos(point.x, point.y - m_scroll);
    }
}

/// Loads the HTML page from the given URL.
void BrowserScene::load(std::string urlString)
{
    try
    {
        URL url(urlString);
        auto body = url.request();
        auto nodes = HTMLParser(body).parse();
        Layout layout(nodes.get());
        m_displayList = layout.getDisplayList();
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
