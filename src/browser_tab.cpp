#include "browser_tab.h"

#include "browser.h"
#include "commit_data.h"
#include "constants.h"
#include "helpers/string_helpers.h"
#include "html_tree/html_parser.h"
#include "html_tree/element.h"
#include "html_tree/text.h"
#include "html_tree/node.h"
#include "html_tree/node_helpers.h"
#include "layout/document_layout.h"
#include "layout/layout.h"
#include "layout/layout_helpers.h"
#include "scripting/js_context.h"
#include "styles/numeric_animation.h"

#include <algorithm>
#include <fstream>
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
      m_logger(Logger("BrowserTab"))
{
    m_scroll = 0;
    m_zoom = 1;
    m_scrollChangedInTab = false;
    m_loaded = false;
    m_needsStyle = false;
    m_needsLayout = false;
    m_needsPaint = false;
    m_needsFocusScroll = false;
    m_needsAccessibility = false;
    focusElement(nullptr);
}

BrowserTab::~BrowserTab() = default;

void BrowserTab::load(URL url, std::optional<std::string> payload)
{
    try
    {
        m_loaded = false;
        focusElement(nullptr);
        m_zoom = 1;
        m_logger.info("loading: " + url.describe());
        m_history.push_back(url);
        auto response = url.request(m_url, payload);
        std::string body = response.errorMessage.has_value() ? response.errorMessage.value() : response.bodyString();
        m_url = url;
        auto resultNodes = HTMLParser(body).parse();
        m_node = std::move(resultNodes);
        setTabTitle();
        loadContentSecurityPolicy(response.headers);
        loadScripts();
        loadStyles();
        setNeedsRender();
        m_scroll = 0;
        m_scrollChangedInTab = true;
        m_loaded = true;
    }
    catch (...)
    {
        m_logger.error("failed to load the browser!");
    }
}

void BrowserTab::setNeedsRender()
{
    m_needsStyle = true;
    browser->setNeedsAnimationFrame(this);
}

void BrowserTab::setNeedsLayout()
{
    m_needsLayout = true;
    browser->setNeedsAnimationFrame(this);
}

void BrowserTab::setNeedsPaint()
{
    m_needsPaint = true;
    browser->setNeedsAnimationFrame(this);
}

int BrowserTab::clampScroll(const int scroll)
{
    int height = ceil(m_document->height + 2 * Constants::VSTEP);
    int maxScroll = height - m_height;
    return std::max(0, std::min(scroll, maxScroll));
}

void BrowserTab::runAnimationFrame(const int scroll)
{
    if (!m_node) return;
    
    if (!m_scrollChangedInTab)
    {
        m_scroll = scroll;
    }

    if (m_js)
    {
        browser->measure.start("script-runRAFHandlers");
        m_js->dispatchRunRAFHandlers();
        browser->measure.stop("script-runRAFHandlers");
    }

    // Run any CSS transition animations.
    for (const auto& node : treeToList(m_node.get()))
    {
        for (const auto& [propertyName, animation] : node->animations)
        {
            auto value = animation->animate();
            if (value.has_value())
            {
                node->styles[propertyName] = value.value();
                m_compositedUpdates.push_back(node);                
                setNeedsPaint();
            }
        }
    }
    
    bool needsComposite = m_needsStyle || m_needsLayout;

    render();
   
    if (m_needsFocusScroll && m_focus)
    {
        scrollTo(m_focus);
    }
    m_needsFocusScroll = false;
    
    std::optional<int> newScroll = std::nullopt;
    if (m_scrollChangedInTab)
    {
        newScroll = m_scroll;
    }
    int documentHeight = 0;
    if (m_document)
    {
        documentHeight = (int)ceil(m_document->height + 2 * Constants::VSTEP);
    }

    std::map<Node*, VisualEffect*> compositedUpdates;
    if (!needsComposite)
    {
        m_compositedUpdates = {};
        for (const auto& node : m_compositedUpdates)
        {
            compositedUpdates[node] = node->blendOp;
        }
    }
    m_compositedUpdates.clear();

    CommitData commitData(
        m_url, 
        newScroll, 
        documentHeight, 
        std::move(m_displayList), 
        compositedUpdates,
        std::move(m_accessibilityTree),
        m_focus
    );
    m_displayList = std::nullopt;
    m_accessibilityTree = nullptr;
    browser->commit(this, commitData);
    m_scrollChangedInTab = false;
}

// Debugging helper
void printLayoutTree(const Layout* layout, int depth = 0)
{
    if (depth == 0)
        std::cout << "Document Layout:" << std::endl;
    std::string indent(depth * 2, ' ');
    std::cout << indent << layout->describe() << std::endl;
    for (const auto& child : layout->children)
        printLayoutTree(child.get(), depth + 1);
}

void printStyles(const CSSRules& rules)
{
    std::stringstream s;
    s << "Styles:" << std::endl;

    for (const auto& rule : rules)
    {
        s << rule.first << ": {";
        std::string seprator = "";
        for (const auto& style : rule.second.styles)
        {
            s << seprator << style.first << ": " << style.second;
            seprator = ", ";
        }
        s << "}" << std::endl;
    }
    std::cout << s.str();
}

void BrowserTab::render()
{
    m_logger.info("rendering");
    browser->measure.start("render");

    if (m_needsStyle)
    {
        //printStyles(m_rules);
        addStyles(*m_node, m_rules);
        m_needsLayout = true;
        m_needsStyle = false;
    }

    if (m_needsLayout)
    {
        m_document = std::make_unique<DocumentLayout>(m_node.get(), m_width);
        m_document->layout(m_zoom);
        //printLayoutTree(m_document.get());
        m_needsAccessibility = true;
        m_needsPaint = true;
        m_needsLayout = false;
    }

    if (m_needsAccessibility)
    {
        m_accessibilityTree = std::make_unique<AccessibilityNode>(m_node.get());
        m_accessibilityTree->build();
        //m_accessibilityTree->printTree();
        m_needsAccessibility = false;
    }

    if (m_needsPaint)
    {
        std::vector<std::unique_ptr<Command>> displayList;
        paintTree(*m_document, displayList);
        m_displayList = std::move(displayList);
        m_needsPaint = false;
    }

    int clampedScroll = clampScroll(m_scroll);
    if (clampedScroll != m_scroll)
    {
        m_scrollChangedInTab = true;
    }
    m_scroll = clampedScroll;

    browser->measure.stop("render");
}

void BrowserTab::resize(int width, int height)
{
    m_width = width;
    m_height = height;
    setNeedsRender();
}

void BrowserTab::zoom(bool increment)
{
    std::string direction = increment ? "in" : "out";
    m_logger.info("zoomed " + direction);

    if (increment)
    {
        m_zoom *= 1.1;
        m_scroll *= 1.1;
    }
    else 
    {
        m_zoom /= 1.1;
        m_scroll /= 1.1;
    }

    m_scrollChangedInTab = true;
    setNeedsRender();
}

void BrowserTab::resetZoom()
{
    m_logger.info("reset zoom");
    m_scroll /= m_zoom;
    m_zoom = 1;
    m_scrollChangedInTab = true;
    setNeedsRender();
}

void BrowserTab::setDarkMode(bool darkMode)
{
    m_darkMode = darkMode;
    setNeedsRender();
}

void BrowserTab::advanceTab()
{
    // Get all the focusable elements.
    std::vector<Element*> focusableElements;
    auto nodes = treeToList(m_node.get());
    for (const auto& node : nodes)
    {
        if (auto element = dynamic_cast<Element*>(node))
        {
            if (isFocusable(element))
            {
                focusableElements.push_back(element);
            }
        }
    }

    // Sort them by their tabindex.
    std::sort(focusableElements.begin(), focusableElements.end(), 
        [](Element* a, Element* b) { 
            return getTabIndex(a) < getTabIndex(b); 
        });

    // Find the index of the currently focused element.
    size_t index = 0;
    auto it = std::ranges::find_if(focusableElements,
        [this](const Element* element) { return m_focus == element; });
    if (it != focusableElements.end())
    {
        index = std::distance(focusableElements.begin(), it) + 1;
    }

    if (index < focusableElements.size())
    {
        focusElement(focusableElements[index]);
        m_logger.info("set focus to " + m_focus->describe());
    }
    else 
    {
        focusElement(nullptr);
        browser->focusAddressBar();
    }

    setNeedsRender();
}

void BrowserTab::enter()
{
    if (!m_focus) return;
    if (m_js->dispatchEvent("click", m_focus)) return;
    activateElement(m_focus);
}

void BrowserTab::activateElement(Element* element)
{
    if (element->tag == "input")
    {
        element->attributes["value"] = "";
        setNeedsRender();
    }
    else if (element->tag == "a" && element->attributes.contains("href"))
    {
        auto href = element->findAttribute("href", "");
        if (!href.empty())
        {
            auto url = m_url.resolve(href);
            load(url);
        }
    }
    else if (element->tag == "button")
    {
        if (m_js->dispatchEvent("click", element)) return;

        Node* node = element;

        while (node)
        {
            if (auto e = dynamic_cast<Element*>(node))
            {
                if (e->tag == "form" && e->attributes.contains("action"))
                {
                    submitForm(element);
                }
            }
            node = node->parent;
        }
    }
}

void BrowserTab::focusElement(Element* element)
{
    if (element && element != m_focus)
        m_needsFocusScroll = true;
    if (m_focus)
        m_focus->isFocused = false;
    m_focus = element;
    if (element)
        element->isFocused = true;
}

/// Adds styles to the given node and its children. 
void BrowserTab::addStyles(Node &node, const CSSRules& rules)
{
    StringMap oldStyle = node.styles;
    node.styles.clear();

    auto inheritedProperties = Constants::INHERITED_PROPERTIES;
    inheritedProperties["color"] = (m_darkMode ? "white" : "black");

    // Apply default styling for inherited properties first
    // since these can be overridden by explicit rules below.
    for (const auto& [property, defaultValue] : inheritedProperties)
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

    // Add style sheet styles to the element nodes based on the 
    // given explicit CSS rules.
    for (const auto& [name, rule] : rules)
    {
        if (name.starts_with(Constants::MEDIA_DARK_PREFIX) && !m_darkMode) continue;
        if (!rule.node->matches(&node)) continue;

        for (const auto& [property, value] : rule.styles)
        {        
            node.styles[property] = value;
        }
    }

    if (auto* element = dynamic_cast<Element*>(&node))
    {
        // Add style attribute styles. Note that the order is important
        // here because the style attribute overrides style sheet values.
        if (element->attributes.contains("style"))
        {
            auto style = element->attributes["style"];
            auto parser = CSSParser(style);
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

    if (!oldStyle.empty())
    {
        auto transitions = diffStyles(oldStyle, node.styles);
        for (const auto& [property, values] : transitions)
        {
            const auto& [oldValue, newValue, numFrames] = values;

            if (property == "opacity")
            {
                setNeedsRender();
                auto animation = std::make_unique<NumericAnimation>(
                    std::stod(oldValue), 
                    std::stod(newValue), 
                    std::stoi(numFrames)
                );
                auto animationResult = animation->animate();
                node.animations[property] = std::move(animation);
                if (animationResult.has_value())
                    node.styles[property] = animationResult.value();
            }
        }
    }

    for (auto& child : node.children)
    {
        addStyles(*child, rules);
    }
}

void BrowserTab::scrollTo(Element* element)
{
    // Find the layout object corresponding to the focused element.
    Layout* obj;
    auto layouts = treeToList<Layout>(m_document.get());

    for (const auto& layout : layouts)
    {
        if (layout->node == element)
        {
            obj = layout;
            break;
        }
    }

    if (!obj) return;
    if (m_scroll < obj->y && obj->y < m_scroll + m_height) return;

    // Now scroll to that layout object.
    auto newScroll = obj->y - Constants::SCROLL_STEP;
    m_logger.info("scrolling to " + std::to_string(newScroll));
    m_scroll = clampScroll(newScroll);
    m_scrollChangedInTab = true;
}

void BrowserTab::loadScripts()
{
    // If we are loading a new page and we already have an existing
    // JSContext, we need to set a discarded flag so that any pending
    // timeouts are discarded.
    if (m_js) 
    {
        m_js->discarded = true;
    }
    m_js = std::make_unique<JSContext>(this);
    auto scripts = getScriptLinks(m_node.get());
    for (const auto& script : scripts)
    {
        auto scriptUrl = m_url.resolve(script);

        // Make sure this script is allowed by the Content Security Policy.
        // If there are no allowed origins defined, then allow the script.
        if (!allowRequest(scriptUrl))
        {
            m_logger.info("blocked script due to CSP: " + script);
            continue;
        }
        try
        {
            auto body = scriptUrl.request(m_url).bodyString();

            // Rather than run the script right away, schedule it to be processed later.
            auto task = std::make_unique<Task<JSContext*, std::string, std::string>>(
                "JSContext::run(\"" + script + "\")", 
                &JSContext::run, 
                m_js.get(), 
                script, 
                body
            );
            taskRunner.scheduleTask(std::move(task));
        }
        catch (...)
        {
            m_logger.error("failed to load script: " + script);
        }
    }
}

CSSRules BrowserTab::getDefaultStyleSheet()
{
    std::ifstream file("assets/styles/browser.css");
    if (!file.is_open())
    {
        m_logger.error("error opening default style sheet!");
        return {};
    }
    std::string fileContents = {std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()};

    auto parser = CSSParser(trim(fileContents));
    return parser.parse();
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
                m_logger.info("blocked style due to CSP: " + link);
                continue;
            }

            auto linkBody = linkUrl.request(m_url).bodyString();
            auto linkResults = CSSParser(trim(linkBody)).parse();

            // The map merge doesn't replace existing values,
            // so we need to erase them first to allow the new
            // values to replace the previous ones.
            for (const auto& [key, value] : linkResults) 
            {
                m_rules.erase(key);
            }
            m_rules.merge(linkResults);
        }
        catch (...)
        {
            m_logger.error("failed to load style: " + link);
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

std::vector<std::string> BrowserTab::getStyleSheetLinks(Node* tree)
{
    std::vector<std::string> links;
    auto nodeList = treeToList<Node>(tree);

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

std::vector<std::string> BrowserTab::getScriptLinks(Node* tree)
{
    std::vector<std::string> links;
    auto nodeList = treeToList<Node>(tree);

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

std::string BrowserTab::getTitle(Node* tree)
{
    auto nodeList = treeToList<Node>(tree);

    for (const auto& node : nodeList)
    {
        if (auto element = dynamic_cast<const Element*>(node))
        {
            if (element->tag == "title" && element->children.size() == 1)
            {
                if (auto text = dynamic_cast<const Text*>(element->children.front().get()))
                {
                    return text->getText();
                }
            }
        }
    }

    // We couldn't find a title so use the URL instead.
    return m_url.describe(false);
}

void BrowserTab::setTabTitle()
{
    auto title = getTitle(m_node.get());
    browser->setTabTitle(title);
}

/// Recursively paints all layout objects, combining the 
/// results into the provided display commands reference.
void BrowserTab::paintTree(const Layout& layout, std::vector<std::unique_ptr<Command>>& displayList)
{
    std::vector<std::unique_ptr<Command>> commands;

    if (layout.shouldPaint())
    {
        commands = layout.paint();
    }

    for (const auto& child : layout.children)
    {
        paintTree(*child, commands);
    }

    if (layout.shouldPaint())
    {
        commands = layout.paintEffects(std::move(commands));
    }

    displayList.insert(
        displayList.end(),
        std::make_move_iterator(commands.begin()),
        std::make_move_iterator(commands.end())
    );
}

void BrowserTab::click(Point point)
{
    focusElement(nullptr);

    auto x = point.x;
    auto y = point.y + m_scroll;    

    auto list = treeToList<Layout>(m_document.get());
    auto locRect = QRect(x, y, 1, 1);

    std::vector<Layout*> objs;
    for (const auto& obj : list)
    {
        if (absoluteBoundsForObj(obj).intersects(locRect))
            objs.push_back(obj);
    }

    if (objs.empty()) return;

    auto last = objs.back();
    Node* elt = last->node;
    while (elt)
    {
        if (auto element = dynamic_cast<Element*>(elt))
        {
            if (isFocusable(element))
            {
                focusElement(element);
                activateElement(element);
                setNeedsRender();
                return;
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
    auto nodeList = treeToList<Node>(element);

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

void BrowserTab::keyPress(std::string text)
{
    if (m_focus && m_focus->tag == "input")
    {
        if (!m_focus->attributes.contains("value"))
            activateElement(m_focus);

        if (m_js->dispatchEvent("keydown", m_focus)) return;
        if (text == "\U0000007f")
        {
            // Handle backspace by removing the last character.
            m_focus->attributes["value"].pop_back();
        }
        else 
        {
            m_focus->attributes["value"] += text;
        }
        setNeedsRender();
    }
}
