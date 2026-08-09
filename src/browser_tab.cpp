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
      m_height(height),
      m_documentSurface(),
      m_visibleSurface(QSize(width, height), QImage::Format_RGBA8888)
{
    m_scroll = 0;
    m_focus = nullptr;
    m_status = LOADING;
}

BrowserTab::~BrowserTab() = default;

void BrowserTab::raster()
{
    auto tabHeight = m_document->height + 2 * Constants::VSTEP;

    if (m_documentSurface.isNull() || m_documentSurface.height() != tabHeight)
    {
        m_documentSurface = QImage(QSize(m_width, tabHeight), QImage::Format_RGBA8888);
    }

    m_documentSurface.fill(Qt::white);

    for (const auto& command : m_displayCommands)
    {
        command->execute(&m_documentSurface);
    }
}

void BrowserTab::draw()
{
    auto visibleRect = QRect(0, m_scroll, m_width, m_height);
    m_visibleSurface = m_documentSurface.copy(visibleRect);

    // The visible surface has changed, so signal that the
    // display needs to be refreshed with the updated surface.
    needsDisplayUpdate(m_visibleSurface);
}

QColor BrowserTab::parseColor(const std::string& color) const 
{
    if (color.starts_with("#") && color.size() == 7)
    {
        int r = std::stoi(color.substr(1, 3));
        int g = std::stoi(color.substr(3, 5));
        int b = std::stoi(color.substr(5, 7));
        return QColor(qRgb(r, g, b));
    }
    else if (Constants::NAMED_COLORS.contains(color))
    {
        return parseColor(Constants::NAMED_COLORS.at(color));
    }
    else 
    {
        return Qt::black;
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
    m_visibleSurface = QImage(QSize(width, height), QImage::Format_RGBA8888);
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
    m_status = LOADING;

    try
    {
        std::cout << "Fetching URL: " << url.describe() << "\n";
        m_history.push_back(url);
        auto response = url.request(m_url, payload);
        std::string body = response.errorMessage.has_value() ? response.errorMessage.value() : response.body;
        m_url = url;
        auto resultNodes = HTMLParser(body).parse();
        m_node = std::move(resultNodes);
        loadContentSecurityPolicy(response.headers);
        loadScripts();
        loadStyles();
        m_status = LOADED;
        render();
    }
    catch (...)
    {
        std::cerr << "Failed to load the browser!\n";
        m_status = ERROR;
    }
}

void BrowserTab::loadScripts()
{
    m_js = std::make_unique<JSContext>(this);
    auto scripts = getScriptLinks(m_node.get());
    for (const auto& script : scripts)
    {
        auto scriptUrl = m_url.resolve(script);

        // Make sure this script is allowed by the Content Security Policy.
        // If there are no allowed origins defined, then allow the script.
        if (!allowRequest(scriptUrl))
        {
            std::cout << "Blocked script due to CSP: " << script << std::endl;
            continue;
        }
        try
        {
            auto body = scriptUrl.request(m_url).body;
            m_js->run(script, body);
        }
        catch (...)
        {
            std::cout << "Failed to load script: " << script << std::endl;
        }
    }
}

void BrowserTab::loadStyles()
{
    m_rules = getDefaultStyleSheet();
    auto links = getStyleSheetLinks(m_node.get());
    for (const auto& link : links)
    {
        try 
        {
            auto linkUrl = m_url.resolve(link);

            // Make sure this style is allowed by the Content Security Policy.
            // If there are no allowed origins defined, this allow the style.
            if (!allowRequest(linkUrl))
            {
                std::cout << "Blocked style due to CSP: " << link << std::endl;
                continue;
            }

            auto linkBody = linkUrl.request(m_url).body;
            auto linkResults = CSSParser(trim(linkBody)).parse();
            m_rules.merge(linkResults);
        }
        catch (...)
        {
            std::cout << "Failed to load style: " << link << std::endl;
        }
    }
}

bool BrowserTab::allowRequest(const URL& url)
{
    auto origin = url.origin();
    auto it = find(m_allowedOrigins.begin(), m_allowedOrigins.end(), origin);
    bool containsUrl = it != m_allowedOrigins.end();
    return m_allowedOrigins.empty() || containsUrl;
}

void BrowserTab::loadContentSecurityPolicy(std::map<std::string, std::string, CaseInsensitiveLess> headers)
{
    m_allowedOrigins.clear();

    if (headers.contains("content-security-policy"))
    {
        auto policy = split(headers["content-security-policy"], " ", false);
        if (policy.size() > 0 && policy[0] == "default-src")
        {
            for (const auto& origin : policy) 
            {
                if (origin == "default-src")
                {
                    continue;
                }
                auto url = URL(origin).origin();
                m_allowedOrigins.push_back(url);
            }
        }
    }
}

void BrowserTab::render()
{
    if (m_status != LOADED) return;
    auto sortedRules = sort(m_rules);
    addStyles(*m_node, sortedRules);
    m_document = std::make_unique<DocumentLayout>(m_node.get(), m_width);
    m_document->layout();
    m_displayCommands.clear();
    paintTree(m_document.get(), m_displayCommands);
    raster();
    draw();
}

void BrowserTab::click(Point point)
{
    if (m_status != LOADED) return;

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
