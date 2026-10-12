#include "frame.h"

#include "helpers/string_helpers.h"
#include "html_tree/html_parser.h"
#include "html_tree/node_helpers.h"
#include "html_tree/text.h"
#include "layout/block_layout.h"
#include "layout/layout_helpers.h"
#include "layout/protected_field.h"
#include "styles/numeric_animation.h"
#include "browser.h"
#include "browser_tab.h"
#include "constants.h"

#include <fstream>

Frame::Frame(BrowserTab* tab, Frame* parentFrame)
    : m_tab(tab),
      m_parentFrame(parentFrame),
      m_url(""),
      m_logger(Logger("Frame"))
{
    m_loaded = false;
    m_needsLayout = false;
    m_needsStyle = false;
    m_needsPaint = false;
    m_width = 0;
    m_height = 0;
    scroll = 0;
    scrollChangedInFrame = false;
    needsFocusScroll = false;

    // Give this frame an identifier, a "window ID".
    windowId = m_tab->windowIdToFrame.size();
    m_tab->windowIdToFrame.emplace_back(windowId, this);
}

void Frame::load(URL url, std::optional<std::string> payload)
{
    m_loaded = false;
    m_logger.info("loading: " + url.describe());
    scroll = 0;
    scrollChangedInFrame = true;

    auto response = url.request(m_url, payload);
    auto body = response.errorMessage.has_value() ? response.errorMessage.value() : response.bodyString();
    m_url = url;

    if (!m_parentFrame)
    {
        m_tab->addHistory(m_url);
    }

    loadContentSecurityPolicy(response.headers);
    parseHtml(body);
    loadScripts();
    loadStyles();
    loadImages();
    loadIFrames();
    document = std::make_unique<DocumentLayout>(m_node.get(), this);
    setNeedsRender();
    m_tab->setTabAddress(url.describe(), getTitle(m_node.get()));
    m_loaded = true;
}

void Frame::render()
{
    m_logger.info("rendering");

    if (m_needsStyle)
    {
        //printStyles(m_rules);
        addStyles(*m_node, m_rules);
        m_needsLayout = true;
        m_needsStyle = false;
    }

    if (m_needsLayout)
    {
        document->layout(m_width, m_tab->getZoom());
        //printLayoutTree(document.get());
        m_tab->setNeedsAccessibility(true);
        m_needsPaint = true;
        m_needsLayout = false;
    }

    int clampedScroll = clampScroll(scroll);
    if (clampedScroll != scroll)
    {
        scrollChangedInFrame = true;
    }
    scroll = clampedScroll;
}

void Frame::setNeedsRender()
{
    m_needsStyle = true;
    if (m_tab)
    {
        m_tab->setNeedsAccessibility(true);
        m_tab->setNeedsPaint();
    }
}

void Frame::setNeedsLayout()
{
    m_needsLayout = true;
    if (m_tab)
    {
        m_tab->setNeedsAccessibility(true);
        m_tab->setNeedsPaint();
    }
}

void Frame::parseHtml(const std::string& body)
{
    auto parser = HTMLParser(body);
    auto resultNodes = parser.parse();
    //parser.printTree(resultNodes.get());
    m_node = std::move(resultNodes);
}

bool Frame::allowRequest(const URL& url)
{
    auto origin = url.origin();
    auto it = find(m_allowedOrigins.begin(), m_allowedOrigins.end(), origin);
    bool containsUrl = it != m_allowedOrigins.end();
    return m_allowedOrigins.empty() || containsUrl;
}

void Frame::loadContentSecurityPolicy(std::map<std::string, std::string, CaseInsensitiveLess> headers)
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

void Frame::loadScripts()
{
    // If we are loading a new page and we already have an existing
    // JSContext, we need to set a discarded flag so that any pending
    // timeouts are discarded.
    if (js) 
    {
        js->discarded = true;
    }
   
    js = nullptr;
    js = m_tab->getJs(m_url);
    if (!js)
        m_logger.error("failed to get JSContext!");
    js->addWindow(this);
    
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
            auto task = std::make_unique<Task<JSContext*, std::string, std::string, int>>(
                "JSContext::run(\"" + script + "\")", 
                &JSContext::run, 
                js, 
                script, 
                body,
                windowId
            );
            taskRunner.scheduleTask(std::move(task));
        }
        catch (...)
        {
            m_logger.error("failed to load script: " + script);
        }
    }
}

void Frame::loadStyles()
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

void Frame::loadImages()
{
    auto images = treeToElementList(m_node.get(), "img");
    for (const auto& image : images)
    {
        auto src = image->findAttribute("src");
        if (src.has_value())
        {
            auto imageUrl = m_url.resolve(src.value());
            if (!allowRequest(imageUrl))
            {
                m_logger.error("blocked image due to CSP: " + imageUrl.describe());
                continue;
            }
            auto response = imageUrl.request(m_url);
            QImage img;
            if (img.loadFromData(response.body))
            {
                image->image = std::make_unique<QImage>(img);
            }
            else 
            {
                image->image = std::make_unique<QImage>(":/assets/images/broken_image.png");
            }
        }
    }
}

void Frame::loadIFrames()
{
    auto iframes = treeToElementList(m_node.get(), "iframe");

    for (auto& iframe : iframes)
    {
        auto src = iframe->findAttribute("src");
        if (!src.has_value()) continue;
        auto documentUrl = m_url.resolve(*src);
        if (!allowRequest(documentUrl))
        {
            m_logger.info("blocked iframe due to CSP: " + documentUrl.describe());
            iframe->frame = nullptr;
            continue;
        }
        iframe->frame = std::make_unique<Frame>(m_tab, this);

        // Since iframes can be slow to load, load them asynchronously.
        auto task = std::make_unique<Task<Frame*, URL, std::optional<std::string>>>(
            "Frame::load(\"" + m_url.describe() + "\")", 
            &Frame::load,
            iframe->frame.get(),
            documentUrl,
            std::nullopt
        );
        taskRunner.scheduleTask(std::move(task));
    }
}

void Frame::initStyles(Node& node)
{
    // Create the styles map, which allows us to freeze each style's depenencies.
    auto& styles = node.styles;
    for (auto const& [property, value] : Constants::CSS_PROPERTIES)
    {
        std::vector<Invalidatable*> dependencies;
        if (node.parent && Constants::INHERITED_PROPERTIES.contains(property))
        {
            dependencies = {&node.parent->styles[property]};
        }

        auto [it, inserted] = styles.try_emplace(property);
        it->second.initialize(&node, property, {}, dependencies);
    }
}

/// Adds styles to the given node and its children. 
void Frame::addStyles(Node& node, const CSSRules& rules)
{
    if (node.styles.empty())
        initStyles(node);

    bool needsStyle = false;
    for (const auto& [property, field] : node.styles)
    {
        if (field.isDirty())
        {
            needsStyle = true;
            break;
        }
    }

    if (!needsStyle)
    {
        for (auto& child : node.children)
        {
            addStyles(*child, rules);
        }
        return;
    }

    StringMap oldStyles;
    for (const auto& [property, field] : node.styles)
    {
        oldStyles.emplace(property, field.getValue());
    }

    StringMap newStyles = Constants::CSS_PROPERTIES;
    auto darkMode = m_tab->isDarkMode();

    auto inheritedProperties = Constants::INHERITED_PROPERTIES;
    inheritedProperties["color"] = (darkMode ? "white" : "black");

    // Apply default styling for inherited properties first
    // since these can be overridden by explicit rules below.
    for (const auto& [property, defaultValue] : inheritedProperties)
    {
        if (node.parent)
        {
            auto parentValue = node.parent->styles[property].read(&node.styles[property]);
            newStyles[property] = parentValue;
        }
        else 
        {
            newStyles[property] = defaultValue;
        }
    }

    // Add style sheet styles to the element nodes based on the 
    // given explicit CSS rules.
    for (const auto& [name, rule] : rules)
    {
        if (name.starts_with(Constants::MEDIA_DARK_PREFIX) && !darkMode) continue;
        if (!rule.node->matches(&node)) continue;

        for (const auto& [property, value] : rule.styles)
        {        
            newStyles[property] = value;
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
                newStyles[property] = value;
            }
        }
        
        // Handle font-size percentages.
        if (newStyles["font-size"].ends_with('%'))
        {
            std::string parentFontSize;

            if (auto* parent = dynamic_cast<Element*>(element->parent))
            {
                parentFontSize = parent->styles["font-size"].read(&node.styles["font-size"]);
            }
            else 
            {
                parentFontSize = Constants::INHERITED_PROPERTIES.at("font-size");
            }

            float nodePercent = percentToFloat(newStyles["font-size"]);
            float parentPixels = pixelsToFloat(parentFontSize);
            newStyles["font-size"] = std::format("{}px", nodePercent * parentPixels);
        }
    }

    if (!oldStyles.empty())
    {
        auto transitions = diffStyles(oldStyles, newStyles);
        for (const auto& [property, values] : transitions)
        {
            auto& [oldValue, newValue, numFrames] = values;

            if (property == "opacity")
            {
                setNeedsRender();
                auto animation = std::make_unique<NumericAnimation>(
                    std::stod(oldValue == "" ? "0" : oldValue), 
                    std::stod(newValue), 
                    std::stoi(numFrames)
                );
                auto animationResult = animation->animate();
                node.animations[property] = std::move(animation);
                if (animationResult.has_value())
                    newStyles[property] = animationResult.value();
            }
        }
    }

    for (auto& [property, field] : node.styles)
    {
        field.set(newStyles[property]);
    }

    for (auto& child : node.children)
    {
        addStyles(*child, rules);
    }
}

CSSRules Frame::getDefaultStyleSheet()
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

std::vector<std::string> Frame::getStyleSheetLinks(Node* tree)
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

std::vector<std::string> Frame::getScriptLinks(Node* tree)
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

std::string Frame::getTitle(Node* tree)
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

void Frame::dispatchRAF(int windowId)
{
    js->dispatchRAF(windowId);
}

void Frame::advanceTab()
{
    if (!m_node) return;
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
        [this](const Element* element) { return m_tab->getFocus() == element; });
    if (it != focusableElements.end())
    {
        index = std::distance(focusableElements.begin(), it) + 1;
    }

    if (index < focusableElements.size())
    {
        focusElement(focusableElements[index]);
#if VERBOSE_DEBUG_OUTPUT        
        m_logger.info("set focus to " + m_tab->getFocus()->describe());
#endif
    }
    else 
    {
        focusElement(nullptr);
        m_tab->browser->focusAddressBar();
    }

    setNeedsRender();
}

void Frame::focusElement(Element* element)
{
    auto focus = m_tab->getFocus();
    if (element && element != focus)
        needsFocusScroll = true;
    if (focus)
    {
        focus->isFocused = false;
        if (element)
            dirtyStyle(focus);
    }
    auto focusedFrame = m_tab->getFocusedFrame();
    if (focusedFrame && focusedFrame != this)
        focusedFrame->setNeedsRender();
    m_tab->setFocus(element);
    m_tab->setFocusedFrame(this);
    if (element)
    {
        element->isFocused = true;
        dirtyStyle(element);
    }
    setNeedsRender();
}

void Frame::activateElement(Element* element)
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
        Node* node = element;

        while (node)
        {
            if (auto e = dynamic_cast<Element*>(node))
            {
                if (e->tag == "form" && e->attributes.contains("action"))
                {
                    submitForm(e);
                    return;
                }
            }
            node = node->parent;
        }
    }
}

void Frame::submitForm(Element* element)
{
    if (js->dispatchEvent("submit", element, windowId)) return;

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

void Frame::scrollDown()
{
    scroll = clampScroll(scroll + Constants::SCROLL_STEP);
    scrollChangedInFrame = true;
}

void Frame::keyPress(std::string text)
{
    auto focus = m_tab->getFocus();
    if (!focus) return;

    if (focus->tag == "input")
    {
        if (!focus->attributes.contains("value"))
            activateElement(focus);

        if (js->dispatchEvent("keydown", focus, windowId)) 
            return;

        if (text == "\U0000007f")
        {
            // Handle backspace by removing the last character.
            focus->attributes["value"].pop_back();
        }
        else 
        {
            focus->attributes["value"] += text;
        }
        setNeedsRender();
    }
    else if (focus->attributes.contains("contenteditable"))
    {
        auto list = treeToList<Node>(focus);
        std::vector<Text*> textNodes;
        for (const auto& item : list)
        {
            if (auto* text = dynamic_cast<Text*>(item))
            {
                textNodes.push_back(text);
            }
        }

        Text* lastText;

        if (!textNodes.empty())
        {
            lastText = textNodes.back();
        }
        else 
        {
            auto text = std::make_unique<Text>("", focus);
            lastText = text.get();
            focus->children.push_back(std::move(text));
        }

        lastText->appendText(text);

        // Rebuild this block's inline children, and mark every
        // ancestor so DocumentLayout will actually visit this node.
        auto obj = focus->layoutObject;
        while (obj && !dynamic_cast<BlockLayout*>(obj))
        {
            obj = obj->parent;
        }
        obj->children.mark();
        setNeedsRender();
    }
}

void Frame::click(Point point)
{
    focusElement(nullptr);

    auto x = point.x;
    auto y = point.y + scroll;

    auto list = treeToList<Layout>(document.get());
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

    if (elt && js->dispatchEvent("click", elt, windowId)) return;

    while (elt)
    {
        if (auto element = dynamic_cast<Element*>(elt))
        {
            if (element->tag == "iframe")
            {
                auto absBounds = absoluteBoundsForObj(element->layoutObject);
                auto border = dpx(1, element->layoutObject->zoom.get());
                auto newX = x - absBounds.left() - border;
                auto newY = y - absBounds.top() - border;
                element->frame->click(Point(newX, newY));
                return;
            }
            else if (isFocusable(element))
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

void Frame::scrollTo(Element* element)
{
    // Find the layout object corresponding to the focused element.
    Layout* obj;
    auto layouts = treeToList<Layout>(document.get());

    for (const auto& layout : layouts)
    {
        if (layout->node == element)
        {
            obj = layout;
            break;
        }
    }

    if (!obj) return;

    auto y = obj->y.getValue();
    if (scroll < y && y < scroll + m_height) return;

    // Now scroll to that layout object.
    auto newScroll = y - Constants::SCROLL_STEP;
    scroll = clampScroll(newScroll);
    m_logger.info("scrolling to " + std::to_string(scroll));
    scrollChangedInFrame = true;
}

void Frame::setFrameSize(double width, double height)
{
    m_width = width;
    m_height = height;
}

int Frame::clampScroll(const int scroll)
{
    int height = ceil(document->height.get() + 2 * Constants::VSTEP);
    int maxScroll = height - m_height;
    return std::max(0, std::min(scroll, maxScroll));
}

Frame* Frame::getTabRootFrame()
{
    return m_tab->getRootFrame();
}
