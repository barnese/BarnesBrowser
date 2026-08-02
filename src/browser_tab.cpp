#include "browser_tab.h"

#include "constants.h"
#include "helpers/string_helpers.h"
#include "html_tree/html_parser.h"
#include "html_tree/element.h"
#include "html_tree/node.h"
#include "layout/document_layout.h"
#include "layout/layout.h"
#include "scripting/js_context.h"

#include <fstream>
#include <iostream>
#include <QFont>
#include <QGraphicsTextItem>
#include <iterator>
#include <memory>
#include <optional>
#include <vector>

BrowserTab::BrowserTab(int width, int height)
    : m_url(""),
      m_width(width),
      m_height(height)
{
    m_scroll = 0;

    // Set the background to white.
    setBackgroundBrush(Qt::white);

    m_focus = nullptr;
}

BrowserTab::~BrowserTab() = default;

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
    if (layout->shouldPaint())
    {
        auto paintList = layout->paint();
        commands.insert(
            commands.end(), 
            std::make_move_iterator(paintList.begin()), 
            std::make_move_iterator(paintList.end())
        );
    }
    for (const auto& child : layout->children)
    {
        paintTree(child.get(), commands);
    }
}

/// Resizes the scene and reconstructs the layout.
void BrowserTab::resize(int width, int height)
{
    m_width = width;
    m_height = height;
    setSceneRect(QRect(0, 0, width, height));
    render();
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
    std::ifstream file("assets/styles/browser.css");
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
std::vector<const Node*> BrowserTab::nodeTreeToList(const Node* tree, std::vector<const Node*>& list) const
{
    list.push_back(tree);
    for (const auto& child : tree->getChildren())
    {
        nodeTreeToList(child.get(), list);
    }
    return list;
}

/// Mutable version of converting a tree into a list.
std::vector<Node*> BrowserTab::nodeTreeToList(Node* tree, std::vector<Node*>& list)
{
    list.push_back(tree);
    for (auto& child : tree->children)
    {
        nodeTreeToList(child.get(), list);
    }
    return list;
}

/// Convenience function to hide implementation details.
std::vector<const Node*> BrowserTab::nodeTreeToList() const 
{
    std::vector<const Node*> list;
    nodeTreeToList(m_node.get(), list);
    return list;
}

/// Mutable convenience function to convert a tree to a list.
std::vector<Node*> BrowserTab::nodeTreeToList()
{
    std::vector<Node*> list;
    nodeTreeToList(m_node.get(), list);
    return list;
}

std::vector<Layout*> layoutTreeToList(Layout* tree, std::vector<Layout*> &list)
{
    list.push_back(tree);
    for (const auto& child : tree->children)
    {
        layoutTreeToList(child.get(), list);
    }
    return list;
}

std::vector<std::string> BrowserTab::getStyleSheetLinks(const Node* tree)
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

std::vector<std::string> BrowserTab::getScriptLinks(const Node* tree)
{
    std::vector<std::string> links;
    std::vector<const Node*> nodeList;
    nodeList = nodeTreeToList(tree, nodeList);

    for (const auto& node : nodeList)
    {
        if (auto element = dynamic_cast<const Element*>(node))
        {
            auto attributes = element->attributes;
            if (element->tag == "script" && attributes.contains("src"))
            {
                links.push_back(attributes["src"]);
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

void BrowserTab::load(URL url, std::optional<std::string> payload)
{
    try
    {
        std::cout << "Fetching URL: " << url.describe() << "\n";
        m_url = url;
        m_history.push_back(url);
        auto body = m_url.request(payload);
        auto resultNodes = HTMLParser(body).parse();
        m_node = std::move(resultNodes);
        loadScripts();
        loadStyles();
        render();
    }
    catch (...)
    {
        std::cerr << "Failed to load the browser!\n";
    }
}

void BrowserTab::loadScripts()
{
    m_js = std::make_unique<JSContext>(this);
    auto scripts = getScriptLinks(m_node.get());
    for (const auto& script : scripts)
    {
        auto scriptUrl = m_url.resolve(script);
        try
        {
            auto body = scriptUrl.request();
            m_js->run(script, body);
        }
        catch (...)
        {
            continue;
        }
    }
}

void BrowserTab::loadStyles()
{
    m_rules = getDefaultStyleSheet();
    auto links = getStyleSheetLinks(m_node.get());
    for (const auto& link : links)
    {
        auto linkUrl = m_url.resolve(link);
        auto linkBody = linkUrl.request();
        auto linkResults = CSSParser(trim(linkBody)).parse();
        m_rules.merge(linkResults);
    }
}

void BrowserTab::render()
{
    auto sortedRules = sort(m_rules);
    addStyles(*m_node, sortedRules);
    m_document = std::make_unique<DocumentLayout>(m_node.get(), m_width);
    m_document->layout();
    m_displayCommands.clear();
    paintTree(m_document.get(), m_displayCommands);
    draw();
}

void BrowserTab::click(Point point)
{
    auto x = point.x;
    auto y = point.y + m_scroll;

    std::vector<Layout*> list;
    list = layoutTreeToList(m_document.get(), list);

    std::vector<Layout*> filteredList;
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

    if (m_focus)
    {
        m_focus->isFocused = false;
    }

    auto last = filteredList.back();
    Node* elt = last->node;
    while (elt != nullptr)
    {
        if (auto element = dynamic_cast<Element*>(elt))
        {
            if (element->tag == "a" && element->attributes.contains("href"))
            {
                if (m_js->dispatchEvent("click", elt)) return;

                auto it = element->attributes.find("href");
                if (it != element->attributes.end())
                {
                    auto url = m_url.resolve(it->second);
                    load(url);
                    linkClicked(url);
                    break;
                }
            } 
            else if (element->tag == "input")
            {
                if (m_js->dispatchEvent("click", elt)) return;

                // Clicking an input element should clear its value.
                element->attributes["value"] = "";
                m_focus = element;
                element->isFocused = true;
                render();
                break;
            }
            else if (element->tag == "button")
            {
                if (m_js->dispatchEvent("click", elt)) return;

                // Find the button's form by walking up the HTML tree.
                // If we find it, submit the form.
                Node* n = element;
                while (n != nullptr)
                {
                    if (auto e = dynamic_cast<Element*>(n)) 
                    {
                        if (e->tag == "form" && e->attributes.contains("action"))
                        {
                            submitForm(e);
                            return;
                        }
                    }
                    n = n->parent;
                }
            }
        }
        elt = elt->parent;
    }
    render();
}

/// Finds all the input elements, encodes their values 
/// and submits them in a POST request.
void BrowserTab::submitForm(Element* element)
{
    if (m_js->dispatchEvent("submit", element)) return;

    std::vector<const Element*> inputs;
    std::vector<const Node*> nodeList;
    nodeList = nodeTreeToList(element, nodeList);

    for (const auto& item : nodeList) 
    {    
        if (auto element = dynamic_cast<const Element*>(item))
        {
            if (element->tag == "input" && 
                element->attributes.contains("name"))
            {
                inputs.push_back(element);
            }
        }
    }

    std::string body = "";

    for (auto& input : inputs)
    {
        auto name = input->findAttribute("name", "");
        auto value = input->findAttribute("value", "");

        if (!body.empty())
        {
            body += "&";
        }

        body += urlEncode(name) + "=" + urlEncode(value);
    }
        
    auto actionUrl = element->findAttribute("action", "");
    auto url = m_url.resolve(actionUrl);
    load(url, body);
}

std::optional<URL> BrowserTab::goBack()
{
    if (m_history.size() <= 1)
    {
        return std::nullopt;
    }

    m_history.pop_back();
    auto url = m_history.back();
    m_history.pop_back();
    load(url);
    return url;
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

void BrowserTab::keyPress(std::string text)
{
    if (m_js->dispatchEvent("keydown", m_focus)) return;

    if (m_focus)
    {
        if (text == "\U0000007f")
        {
            // Handle backspace by removing the last character.
            m_focus->attributes["value"].pop_back();
        }
        else 
        {
            m_focus->attributes["value"] += text;
        }
        render();
    }
}
