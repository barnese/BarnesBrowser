#include "browser_tab.h"

#include "constants.h"
#include "helpers/string_helpers.h"
#include "html_tree/html_parser.h"
#include "html_tree/element.h"
#include "layout/document_layout.h"
#include "layout/layout.h"
#include "styles/css_parser.h"

#include <fstream>
#include <iostream>
#include <QFont>
#include <QGraphicsTextItem>
#include <iterator>
#include <memory>
#include <vector>

BrowserTab::BrowserTab(int width, int height)
    : m_url(""),
      m_width(width),
      m_height(height)
{
    m_scroll = 0;

    // Set the background to white.
    setBackgroundBrush(Qt::white);
}

/// Draws the display list to the screen.
/// Draw operates using screen coordinates, which are the
/// visible coordinates on the screen based on the current
/// scroll value.
void BrowserTab::draw()
{
    // Erase the entire scene so we can redraw its contents.
    clear();

    for (const auto& command : m_displayCommands)
    {
        // Prevent drawing off-screen content.
        if (command->top > m_scroll + m_height ||
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

/// Reconstructs the document layout and redraws its contents.
void BrowserTab::layout()
{
    m_document = std::make_unique<DocumentLayout>(m_node.get(), m_width);
    m_document->layout();
    m_displayCommands.clear();
    paintTree(m_document.get(), m_displayCommands);
    draw();
}

/// Resizes the scene and reconstructs the layout.
void BrowserTab::resize(int width, int height)
{
    m_width = width;
    m_height = height;
    setSceneRect(QRect(0, 0, width, height));
    layout();
}

/// Adds styles to the given node and its children. 
void addStyles(Node &node, const SortedCSSRules& rules)
{
    // Apply default styling for inherited properties first
    // since these can be overridden by explicit rules below.
    for (const auto& [property, defaultValue] : Constants::INHERITED_PROPERTIES)
    {
        if (node.parent)
        {
            node.styles[property] = node.parent->styles[property];
        }
        else 
        {
            node.styles[property] = defaultValue;
        }
    }

    if (auto* element = dynamic_cast<Element*>(&node))
    {
        // Add style sheet styles to the element nodes based on the 
        // given explicit CSS rules.
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
        
        // Handle font-size percentages.
        if (element->styles["font-size"].ends_with('%'))
        {
            std::string parentFontSize;

            if (auto* parent = dynamic_cast<Element*>(element->parent))
            {
                parentFontSize = parent->styles["font-size"];
            }
            else 
            {
                parentFontSize = Constants::INHERITED_PROPERTIES.at("font-size");
            }

            float nodePercent = percentToFloat(element->styles["font-size"]);
            float parentPixels = pixelsToFloat(parentFontSize);
            element->styles["font-size"] = std::format("{}px", nodePercent * parentPixels);
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

/// Converts a tree into a list of nodes.
std::vector<const Node*> nodeTreeToList(const Node* tree, std::vector<const Node*>& list)
{
    list.push_back(tree);
    for (const auto& child : tree->getChildren())
    {
        nodeTreeToList(child.get(), list);
    }
    return list;
}

std::vector<const Layout*> layoutTreeToList(const Layout* tree, std::vector<const Layout*> &list)
{
    list.push_back(tree);
    for (const auto& child : tree->children)
    {
        layoutTreeToList(child.get(), list);
    }
    return list;
}

std::vector<std::string> getStyleSheetLinks(const Node* tree)
{
    std::vector<std::string> links;
    std::vector<const Node*> nodeList;
    nodeList = nodeTreeToList(tree, nodeList);

    for (const auto& node : nodeList) 
    {    
        if (auto element = dynamic_cast<const Element*>(node))
        {
            auto attributes = element->attributes;
            if (element->tag == "link" && 
                attributes["rel"] == "stylesheet" &&
                attributes.contains("href"))
            {
                links.push_back(attributes["href"]);
            }
        }
    }

    return links;
}

/// Loads the HTML page from the given URL.
void BrowserTab::load(std::string urlString)
{
    URL url(urlString);
    load(url);
}

void BrowserTab::load(URL url)
{
    try
    {
        std::cout << "Fetching URL: " << url.describe() << "\n";
        m_url = url;
        auto body = m_url.request();
        auto resultNodes = HTMLParser(body).parse();
        m_node = std::move(resultNodes);
        auto rules = getDefaultStyleSheet();
        auto links = getStyleSheetLinks(m_node.get());
        for (const auto& link : links)
        {
            auto linkUrl = m_url.resolve(link);
            auto linkBody = linkUrl.request();
            auto linkResults = CSSParser(trim(linkBody)).parse();
            rules.merge(linkResults);
        }
        auto sortedRules = sort(rules);
        addStyles(*m_node, sortedRules);
        layout();
    }
    catch (...)
    {
        std::cerr << "Failed to load the browser!\n";
    }
}

void BrowserTab::click(Point point)
{
    auto x = point.x;
    auto y = point.y + m_scroll;

    std::vector<const Layout*> list;
    list = layoutTreeToList(m_document.get(), list);

    std::vector<const Layout*> filteredList;
    for (const auto& layout : list)
    {
        if (layout->x <= x && x < layout->x + layout->width &&
            layout->y <= y && y < layout->y + layout->height)
        {
            filteredList.push_back(layout);
        }
    }

    if (filteredList.empty())
    {
        return;
    }

    auto last = filteredList.back();
    const Node* elt = last->node;
    while (elt != nullptr)
    {
        if (auto element = dynamic_cast<const Element*>(elt))
        {
            if (element->tag == "a" && element->attributes.contains("href"))
            {
                auto it = element->attributes.find("href");
                if (it != element->attributes.end())
                {
                    auto url = m_url.resolve(it->second);
                    load(url);
                    break;
                }
            }
        }
        elt = elt->parent;
    }
}

void BrowserTab::scrollDown()
{
    // Scroll down, but prevent scrolling past the bottom of the document.
    int verticalPadding = 2 * Constants::VSTEP;
    int maxY = std::max(m_document->height + verticalPadding - m_height, 0);
    m_scroll = std::min(m_scroll + Constants::SCROLL_STEP, maxY);
    draw();
}

void BrowserTab::scrollUp()
{
    m_scroll -= Constants::SCROLL_STEP;
    if (m_scroll < 0) 
    {
        m_scroll = 0;
    }
    draw();
}
