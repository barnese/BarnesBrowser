#include "browser_scene.h"

#include "constants.h"
#include "helpers/string_helpers.h"
#include "html_tree/html_parser.h"
#include "html_tree/node.h"
#include "html_tree/element.h"
#include "layout/document_layout.h"
#include "layout/layout.h"
#include "styles/css_parser.h"
#include "url.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <QFont>
#include <QGraphicsTextItem>
#include <iterator>
#include <memory>
#include <vector>

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

    for (const auto& command : m_displayCommands)
    {
        // Prevent drawing off-screen content.
        if (command->top > m_scroll + Constants::HEIGHT ||
            command->bottom < m_scroll)
        {
            continue;
        }

        //std::cout << command->describe() << "\n";
        command->execute(this, m_scroll);
    }
}

/// Recursively paints all layout objects, combining the 
/// results into the provided display commands reference.
void paintTree(const Layout* layout, std::vector<std::unique_ptr<Command>> &commands)
{
    auto paintList = layout->paint();

    commands.insert(
        commands.end(), 
        std::make_move_iterator(paintList.begin()), 
        std::make_move_iterator(paintList.end())
    );

    for (const auto& child : layout->children)
    {
        paintTree(child.get(), commands);
    }
}

/// Adds styles to the given node and its children. 
void addStyles(Node &node, const CSSRules& rules)
{
   if (auto* element = dynamic_cast<Element*>(&node))
   {
        // Add style sheet styles to the element nodes.
        for (const auto& [selector, body] : rules)
        {
            if (!selector->matches(&node))
            {
                continue;
            }

            for (const auto& [property, value] : body)
            {
                element->styles[property] = value;
            }
        }

        // Add style attribute styles. Note that the order is important
        // here because the style attribute overrides style sheet values.
        if (element->attributes.contains("style"))
        {
            auto parser = CSSParser(element->attributes["style"]);
            auto pairs = parser.body();
            for (const auto& [property, value] : pairs)
            {
                element->styles[property] = value;
            }
        }
   } 

   for (auto& child : node.children)
   {
        addStyles(*child, rules);
   }
}

CSSRules getDefaultStyleSheet()
{
    std::ifstream file("assets/browser.css");
    if (!file.is_open())
    {
        std::cerr << "Error opening default style sheet!";
        return {};
    }
    std::string fileContents = {std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()};

    auto parser = CSSParser(trim(fileContents));
    return parser.parse();
}

/// Loads the HTML page from the given URL.
void BrowserScene::load(std::string urlString)
{
    try
    {
        URL url(urlString);
        auto body = url.request();
        auto nodes = HTMLParser(body).parse();
        auto rules = getDefaultStyleSheet();
        addStyles(*nodes, rules);
        m_document = std::make_unique<DocumentLayout>(nodes.get());
        m_document->layout();
        m_displayCommands.clear();
        paintTree(m_document.get(), m_displayCommands);
        draw();
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << "Error: " << error.what() << "\n";
    }
}

void BrowserScene::scrollDown()
{
    // Scroll down, but prevent scrolling past the bottom of the document.
    int verticalPadding = 2 * Constants::VSTEP;
    int maxY = std::max(m_document->height + verticalPadding - Constants::HEIGHT, 0);
    m_scroll = std::min(m_scroll + Constants::SCROLL_STEP, maxY);
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
