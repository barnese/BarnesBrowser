#include "browser_scene.h"
#include "constants.h"
#include "url.h"
#include <iostream>
#include <vector>
#include <QGraphicsTextItem>
#include <QFont>
#include <QFontMetrics>

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

/// Updates the layout's display list for the given display content.
/// Layout operates using page coordinates, which represent 
/// an entire page, not only what is visible on the screen.
void BrowserScene::layout(std::vector<Element> elements)
{
    m_displayList.clear();

    int weight = QFont::Weight::Normal;
    int style = QFont::Style::StyleNormal;
    int cursorX = 0;
    int cursorY = 0;

    for (const auto& element : elements)
    {
        if (element.isTag()) 
        {
            std::string tag = element.getText();

            if (tag == "i")
                style = QFont::Style::StyleItalic;
            else if (tag == "/i")
                style = QFont::Style::StyleNormal;
            else if (tag == "b")
                weight = QFont::Weight::Bold;
            else if (tag == "/b")
                weight = QFont::Weight::Normal;
        }
        else
        {
            QString content(element.getText().c_str());
            auto words = content.split(' ');

            for (auto word : words)
            {
                QFont font("Helvetica", 16, weight, style);
                QFontMetrics metrics(font);          
                auto wordWidth = metrics.horizontalAdvance(word);

                if (cursorX + wordWidth >= Constants::WIDTH)
                {
                    cursorY += metrics.lineSpacing() * 1.25;
                    cursorX = 0;
                }
                Point point(cursorX, cursorY);
                m_displayList[point] = {word, font};

                cursorX += wordWidth + metrics.horizontalAdvance(' ');
            }           
        }            
    }
}

/// Gets the content of an HTML document as a list of tokens.
/// Tokens are objects for the tags and text of the HTML.
std::vector<Element> BrowserScene::lex(std::string body)
{
    std::vector<Element> elements;
    std::string buffer;
    bool inTag = false;

    for (char c : body) 
    {
        if (c == '<') 
        {
            inTag = true;
            if (!buffer.empty())
            {
                elements.emplace_back(Element(buffer, false));
                buffer = "";
            }
        }
        else if (c == '>') 
        {
            inTag = false;
            elements.emplace_back(Element(buffer, true));
            buffer = "";
        }
        else
        {
            buffer += c;
        }
    }

    if (!inTag && !buffer.empty()) 
    {
        elements.emplace_back(Element(buffer, false));
    }

    return elements;
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
