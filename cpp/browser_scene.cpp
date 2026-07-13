#include "browser_scene.h"
#include "constants.h"
#include "layout/document_layout.h"
#include "html_tree/html_parser.h"
#include "url.h"
#include <iostream>
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

/// Recursively paints all layout objects, combining the 
/// results into the provided display list reference.
void paintTree(const Layout* layout, DisplayList &displayList)
{
    auto layoutDisplayList = layout->paint();
    displayList.insert(layoutDisplayList.begin(), layoutDisplayList.end());

    for (const auto& child : layout->children)
    {
        paintTree(child.get(), displayList);
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
        DocumentLayout document(nodes.get());
        document.layout();
        m_displayList = {};
        paintTree(&document, m_displayList);
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
