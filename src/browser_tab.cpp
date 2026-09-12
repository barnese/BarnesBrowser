#include "browser_tab.h"

#include "html_tree/node_helpers.h"
#include "html_tree/text.h"
#include "layout/layout.h"
#include "layout/iframe_layout.h"
#include "scripting/js_context.h"
#include "browser.h"
#include "commit_data.h"
#include "frame.h"

#include <algorithm>
#include <QFont>
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
    m_scrollChangedInTab = false;
    m_zoom = 1;
    m_loaded = false;
    m_needsPaint = false;
    m_needsAccessibility = false;
    m_focus = nullptr;
    m_focusedFrame = nullptr;
    m_rootFrame = nullptr;
}

BrowserTab::~BrowserTab() = default;

void BrowserTab::load(URL url, std::optional<std::string> payload)
{
    browser->measure.start("load");
    try
    {
        m_loaded = false;
        m_logger.info("loading: " + url.describe());
        m_history.push_back(url);
        taskRunner.clearPendingTasks();
        m_rootFrame = std::make_unique<Frame>(this, nullptr, nullptr);
        m_rootFrame->load(url, payload);
        m_rootFrame->setFrameSize(m_width, m_height);
        setTabTitle();
        m_loaded = true;
    }
    catch (...)
    {
        m_logger.error("failed to load the browser!");
    }
    browser->measure.stop("load");
}

void BrowserTab::runAnimationFrame(const int scroll)
{
    if (!m_rootFrame) return;

    if (!m_rootFrame->scrollChangedInFrame)
    {
        m_rootFrame->scroll = scroll;
    }

    bool needsComposite = false;

    for (const auto& [windowId, frame] : windowIdToFrame)
    {
        if (!frame->isLoaded()) continue;

        browser->measure.start("script-runRAFHandlers");
        frame->dispatchRAF(windowId);
        browser->measure.stop("script-runRAFHandlers");

        // Run any CSS transition animations.
        for (const auto& node : treeToList(frame->getNode()))
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
     
        if (frame->needsStyle() || frame->needsLayout())
        {
            needsComposite = true;
        }
    }

    render();
   
    if (m_focus && m_focusedFrame->needsFocusScroll)
    {
        m_focusedFrame->scrollTo(m_focus);
        m_focusedFrame->needsFocusScroll = false;
    }

    for (const auto& [windowId, frame] : windowIdToFrame)
    {
        if (frame == m_rootFrame.get()) continue;
        if (frame->scrollChangedInFrame)
        {
            needsComposite = true;
            frame->scrollChangedInFrame = false;
        }
    }

    std::optional<int> newScroll = std::nullopt;
    if (m_rootFrame->scrollChangedInFrame)
    {
        newScroll = m_rootFrame->scroll;
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

    bool rootFrameFocused = !(m_focusedFrame || m_focusedFrame == m_rootFrame.get());

    CommitData commitData(
        m_url, 
        newScroll, 
        rootFrameFocused,
        ceil(m_rootFrame->document->height), 
        std::move(m_displayList), 
        compositedUpdates,
        std::move(m_accessibilityTree),
        m_focus
    );
    m_displayList = std::nullopt;
    m_rootFrame->scrollChangedInFrame = false;
    m_accessibilityTree = nullptr;
    browser->commit(this, commitData);
}

void BrowserTab::setNeedsPaint()
{
    m_needsPaint = true;
    browser->setNeedsAnimationFrame(this);
}

void BrowserTab::setNeedsRenderAllFrames()
{
    for (const auto& [id, frame] : windowIdToFrame)
    {
        frame->setNeedsRender();
    }
}

void BrowserTab::render()
{
    browser->measure.start("render");
    m_logger.info("rendering");

    for (const auto& [id, frame] : windowIdToFrame)
    {
        if (frame->isLoaded())
            frame->render();
    }

    if (m_needsAccessibility)
    {
        auto node = m_rootFrame->getNode();
        m_accessibilityTree = std::make_unique<AccessibilityNode>(node);
        m_accessibilityTree->build();
        //m_accessibilityTree->printTree();
        m_needsAccessibility = false;
        m_needsPaint = true;
    }

    if (m_needsPaint)
    {
        std::vector<std::unique_ptr<Command>> displayList;
        paintTree(*m_rootFrame->document.get(), displayList);
        m_displayList = std::move(displayList);
        m_needsPaint = false;
    }

    browser->measure.stop("render");
}

void BrowserTab::paintTree(const Layout& layout, std::vector<std::unique_ptr<Command>>& displayList)
{
    std::vector<std::unique_ptr<Command>> commands = layout.paint();

    if (dynamic_cast<const IFrameLayout*>(&layout))
    {
        if (const auto element = dynamic_cast<Element*>(layout.node))
        {
            if (element->frame && element->frame->isLoaded())
            {
                paintTree(*element->frame->document.get(), displayList);
            }
        }
    }
    else 
    {
        for (const auto& child : layout.children)
        {
            paintTree(*child, commands);
        }    
    }

    commands = layout.paintEffects(std::move(commands));

    displayList.insert(
        displayList.end(),
        std::make_move_iterator(commands.begin()),
        std::make_move_iterator(commands.end())
    );
}


void BrowserTab::resize(int width, int height)
{
    m_width = width;
    m_height = height;
    setNeedsRenderAllFrames();
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
    setNeedsRenderAllFrames();
}

void BrowserTab::resetZoom()
{
    m_logger.info("reset zoom");
    m_scroll /= m_zoom;
    m_zoom = 1;
    m_scrollChangedInTab = true;
    setNeedsRenderAllFrames();
}

void BrowserTab::setDarkMode(bool darkMode)
{
    m_darkMode = darkMode;
    setNeedsRenderAllFrames();
}

void BrowserTab::advanceTab()
{
    auto frame = m_focusedFrame ? m_focusedFrame : m_rootFrame.get();
    frame->advanceTab();
}

void BrowserTab::enter()
{
    if (!m_focus) return;
    auto frame = m_focusedFrame ? m_focusedFrame : m_rootFrame.get();
    frame->activateElement(m_focus);
}

void BrowserTab::keyPress(std::string text)
{
    auto frame = m_focusedFrame ? m_focusedFrame : m_rootFrame.get();
    frame->keyPress(text);
}

std::unique_ptr<JSContext> BrowserTab::getJs(URL url)
{
    // TODO: This needs to get rewritten.
    return std::make_unique<JSContext>(this);
}


bool BrowserTab::allowRequest(const URL& url)
{
    auto origin = url.origin();
    auto it = find(m_allowedOrigins.begin(), m_allowedOrigins.end(), origin);
    bool containsUrl = it != m_allowedOrigins.end();
    return m_allowedOrigins.empty() || containsUrl;
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
    auto node = m_rootFrame->getNode();
    auto title = getTitle(node);
    browser->setTabTitle(title);
}

void BrowserTab::click(Point point)
{
    render();
    m_rootFrame->click(point);
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
