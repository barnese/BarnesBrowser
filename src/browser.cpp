#include "browser.h"

#include "commands/blend.h"
#include "commands/draw_composited_layer.h"
#include "commands/paint_command.h"
#include "constants.h"
#include "helpers/thread_registry.h"
#include "html_tree/node_helpers.h"

#include <QBoxLayout>

Browser::Browser(QWidget* parent) 
    : QWidget(parent), m_logger(Logger("Browser"))
{
    // Add the current thread to the thread registry,
    // which allows us to have human-readable thread names.
    threadRegistry().add(std::this_thread::get_id(), "Browser Thread");

    m_logger.info("starting");

    auto* layout = new QVBoxLayout(this);
    m_chrome = new Chrome(this);
    m_view = new BrowserView(this);
    layout->addWidget(m_chrome);
    layout->addWidget(m_view);
    layout->setContentsMargins(0, 0, 0, 0);

    connect(m_chrome, &Chrome::newTabRequested, this, &Browser::createNewTab);
    connect(m_chrome, &Chrome::tabChanged, this, &Browser::changeTab);
    connect(m_chrome, &Chrome::addressChanged, this, &Browser::changeAddress);
    connect(m_chrome, &Chrome::navigateBackRequested, this, &Browser::goBack);
    
    connect(m_view, &BrowserView::paintRequested, this, &Browser::handlePaintRequest);
    connect(m_view, &BrowserView::click, this, &Browser::handleClick);
    connect(m_view, &BrowserView::keyPress, this, &Browser::handleKeyPress);
    connect(m_view, &BrowserView::resize, this, &Browser::handleResize);
    connect(m_view, &BrowserView::scrollDown, this, &Browser::handleScrollDown);
    connect(m_view, &BrowserView::scrollUp, this, &Browser::handleScrollUp);
    connect(m_view, &BrowserView::handleEnter, this, &Browser::handleEnter);
    connect(m_view, &BrowserView::handleTab, this, &Browser::handleTab);
    connect(m_view, &BrowserView::toggleAccessibility, this, &Browser::handleAccessibilityToggle);
    connect(m_view, &BrowserView::zoom, this, &Browser::incrementZoom);
    connect(m_view, &BrowserView::resetZoom, this, &Browser::resetZoom);
    connect(m_view, &BrowserView::setDarkMode, this, &Browser::setDarkMode);
    connect(m_view, &BrowserView::goBack, this, &Browser::goBack);
    connect(m_view, &BrowserView::focusAddressBar, this, &Browser::focusAddressBar);
    connect(m_view, &BrowserView::newTab, this, &Browser::createNewTab);
    connect(m_view, &BrowserView::cycleTabs, this, &Browser::cycleTabs);

    connect(&m_timer, &QTimer::timeout, this, &Browser::mainTimerFired);
    m_timer.start(10);

    m_animationTimer = nullptr;
    m_needsAnimationFrame = false;
    m_needsComposite = false;
    m_needsRaster = false;
    m_needsDraw = false;
    m_activeTabHeight = 0;
    m_activeTabScroll = 0;
    m_activeTabDisplayList.clear();
    m_darkMode = false;
    m_focus = std::nullopt;
    m_needsAccessibility = false;
    m_accessibilityEnabled = false;
    m_hasSpokenDocument = false;
    m_tabFocus = nullptr;
    m_lastTabFocus = nullptr;
}

Browser::~Browser()
{
    measure.finish();
    for (const auto& tab : m_tabs)
    {
        tab->taskRunner.setNeedsQuit();
    }
}

void Browser::mainTimerFired()
{
    // Continuously check if we need to raster/draw 
    // or schedule an animation frame.
    compositeRasterAndDraw();
    scheduleAnimationFrame();
}

void Browser::render()
{
    if (m_activeTab->isLoaded())
    {
        m_activeTab->runAnimationFrame(m_activeTabScroll);
    }
}

void Browser::commit(BrowserTab* tab, CommitData& commitData)
{
    std::lock_guard lock(m_mutex);
    if (tab == m_activeTab)
    {
        m_activeTabUrl = commitData.url;
        if (commitData.scroll.has_value())
            m_activeTabScroll = commitData.scroll.value();
        m_activeTabHeight = commitData.height;
        if (commitData.displayList.has_value())
        {
            m_activeTabDisplayList = std::move(commitData.displayList.value());
        }
        m_accessibilityTree = std::move(commitData.accessibilityTree);
        m_tabFocus = commitData.focus;
        
        m_compositedUpdates = commitData.compositedUpdates;
        if (m_compositedUpdates.empty())
        {
            setNeedsComposite();
        }
        else 
        {
            setNeedsDraw();
        }
    }
}

void Browser::setNeedsAnimationFrame(BrowserTab* tab)
{
    std::lock_guard lock(m_mutex);
    if (tab == m_activeTab)
    {
        m_needsAnimationFrame = true;
    }
}

void Browser::setNeedsComposite()
{
    m_needsComposite = true;
    m_needsRaster = true;
    m_needsDraw = true;
}

void Browser::setNeedsDraw()
{
    m_needsDraw = true;
}

void Browser::setNeedsRaster()
{
    m_needsRaster = true;
    m_needsDraw = true;
}

void Browser::setNeedsAccessibility()
{
    if (!m_accessibilityEnabled) return;
    m_needsAccessibility = true;
    m_needsDraw = true;
}

void Browser::compositeRasterAndDraw()
{
    std::lock_guard lock(m_mutex);
    if (!m_needsComposite && !m_needsRaster && !m_needsDraw) return;
    m_logger.info("composite rastering and drawing");
    measure.start("compositeRasterAndDraw");
    if (m_needsComposite)
    {
        measure.start("composite");
        composite();
        measure.stop("composite");
    }
    if (m_needsRaster)
    {
        measure.start("raster");
        rasterTab();
        measure.stop("raster");
    }
    if (m_needsDraw)
    {
        measure.start("draw");
        paintDrawList();
        draw();
        measure.stop("draw");
    }
    if (m_needsAccessibility)
    {
        updateAccessibility();
    }
    measure.stop("compositeRasterAndDraw");
    m_needsComposite = false;
    m_needsRaster = false;
    m_needsDraw = false;
}

void Browser::scheduleAnimationFrame()
{
    std::lock_guard lock(m_mutex);

    if (m_needsAnimationFrame && !m_animationTimer)
    {
        m_animationTimer = new QTimer(this);
        m_animationTimer->setSingleShot(true);
        connect(m_animationTimer, &QTimer::timeout, this, [this] {
            std::lock_guard lock(m_mutex);
            m_animationTimer = nullptr;
            m_needsAnimationFrame = false;
            auto task = std::make_unique<Task<BrowserTab*, int>>(
                "BrowserTab::runAnimationFrame",
                &BrowserTab::runAnimationFrame,
                m_activeTab,
                m_activeTabScroll
            );
            m_activeTab->taskRunner.scheduleTask(std::move(task));
       });
        m_animationTimer->start(Constants::REFRESH_RATE_MS);   
    }
}

void Browser::rasterTab()
{
    for (const auto& compositedLayer : m_compositedLayers)
    {
        compositedLayer->raster();
    }
}

void Browser::draw()
{
    // Tell the view we need to update. This will cause the 
    // view to send back a paint request signal with the 
    // painter that we can use to draw.
    m_view->update();
}

void Browser::handlePaintRequest(PaintContext& context)
{
    if (!m_activeTab->isLoaded()) return;

    context.scroll = m_activeTabScroll;

    for (const auto& item : m_drawList)
    {
        item->execute(context);
    }
}

void addParentPointers(std::vector<std::unique_ptr<Command>>& commands, Command* parent = nullptr)
{
    for (auto& command : commands)
    {
        command->parent = parent;
        addParentPointers(command->children, command.get());
    }
}

void Browser::composite()
{
    // To make it easy to access ancestor visual effects and compare 
    // them, add parent pointers to our display list tree.
    addParentPointers(m_activeTabDisplayList);

    m_compositedLayers.clear();

    std::vector<Command*> allCommands;
    std::vector<Command*> nonCompositedCommands;

    for (const auto& command : m_activeTabDisplayList)
    {
        treeToList(command.get(), allCommands);
    }

    for (const auto& cmd : allCommands)
    {
        if ((dynamic_cast<PaintCommand*>(cmd) || !cmd->needsCompositing) &&
            (!cmd->parent || cmd->parent->needsCompositing))
        {
            nonCompositedCommands.push_back(cmd);
        }
    }

    for (auto cmd : nonCompositedCommands)
    {
        bool needsNewLayer = true;

        // Loop backwards through the composited layers.
        for (auto it = m_compositedLayers.crbegin(); it != m_compositedLayers.crend(); ++it)
        {
            if ((**it).canMerge(cmd))
            {
                (**it).add(cmd);
                needsNewLayer = false;
                break;
            }
            else if ((**it).absoluteBounds().intersects(localToAbsolute(cmd, cmd->rect)))
            {
                needsNewLayer = true;
                break;
            }
        }

        if (needsNewLayer)
        {            
            m_compositedLayers.push_back(std::make_unique<CompositedLayer>(cmd));
        }
    }
}

Command* Browser::getLatest(Command* effect)
{
    auto node = effect->node;
    if (!node)
    {
        return effect;
    }
    if (!m_compositedUpdates.contains(node))
    {
        return effect;
    }
    if (!dynamic_cast<Blend*>(effect))
    {
        return effect;
    }
    return m_compositedUpdates[node];
}

void Browser::paintDrawList()
{
    std::map<Command*, Command*> newEffects;

    m_drawList.clear();

    for (const auto& compositedLayer : m_compositedLayers)
    {
        if (!compositedLayer->displayItems.has_value()) 
            continue;

        std::unique_ptr<Command> current =
            std::make_unique<DrawCompositedLayer>(compositedLayer.get());

        auto parent = compositedLayer->displayItems->front()->parent;

        while (parent)
        {
            // Get an updated visual effect from composited updates, if there is one.
            auto newParent = getLatest(parent);

            if (newEffects.contains(newParent))
            {
                newEffects.at(newParent)->children.push_back(std::move(current));
                break;
            }
            else 
            {
                auto cloned = newParent->clone(std::move(current));
                newEffects[newParent] = cloned.get();
                current = std::move(cloned);
                parent = parent->parent;
            }
        }

        if (!parent)
        {
            m_drawList.push_back(std::move(current));
        }
    }
}

void Browser::scheduleLoad(URL url)
{
    auto task = std::make_unique<Task<BrowserTab*, URL, std::optional<std::string>>>(
        "BrowserTab::load(\"" + url.describe(false) + "\")", 
        &BrowserTab::load,
        m_activeTab, 
        url,
        std::nullopt
    );
    m_activeTab->taskRunner.clearPendingTasks();
    m_activeTab->taskRunner.scheduleTask(std::move(task));
}

void Browser::newTab(std::optional<URL> url)
{
    std::lock_guard lock(m_mutex);
    newTabInternal(url);
}

void Browser::newTabInternal(std::optional<URL> url)
{
    auto tab = std::make_unique<BrowserTab>(size().width(), size().height());
    tab->browser = this;
    m_tabs.push_back(std::move(tab));
    setActiveTab(m_tabs.back().get());
    QString address;
    QString tabName;
    if (url.has_value())
    {
        scheduleLoad(url.value());
        address = QString::fromStdString(url.value().describe());
        tabName = address;
    }
    else 
    {
        address = "";
        tabName = "New Tab";
    }
    m_view->setFocus();
    m_chrome->addTab(tabName, address, m_tabs.size() - 1);
}

void Browser::linkClicked(URL url)
{
    std::lock_guard lock(m_mutex);
    QString urlString = QString::fromStdString(url.describe(false));
    m_chrome->setAddress(urlString);
}

void Browser::createNewTab()
{
    newTab();
    focusAddressBar();
}

void Browser::changeTab(size_t index)
{
    if (m_tabs.empty() || index > m_tabs.size())
    {
        return;
    }

    m_logger.info("changing tab to " + std::to_string(index));
    setActiveTab(m_tabs.at(index).get());

    std::lock_guard lock(m_mutex);
    auto task = std::make_unique<Task<BrowserTab*>>(
        "BrowserTab::setNeedsRender",
        &BrowserTab::setNeedsRender,
        m_activeTab
    );
    m_activeTab->taskRunner.scheduleTask(std::move(task));
}

void Browser::setTabTitle(std::string title)
{
    std::lock_guard lock(m_mutex);
    auto activeIndex = getActiveTabIndex();
    m_chrome->setTabName(QString::fromStdString(title), activeIndex);
}

void Browser::setActiveTab(BrowserTab* tab)
{
    m_activeTab = tab;
    m_activeTab->taskRunner.scheduleTask(
        std::make_unique<Task<BrowserTab*, bool>>(
            "BrowserTab::setDarkMode(" + std::to_string(m_darkMode) + ")",
            &BrowserTab::setDarkMode,
            m_activeTab,
            m_darkMode
        )
    );
    m_activeTab->taskRunner.scheduleTask(
        std::make_unique<Task<BrowserTab*>>(
            "BrowserTab::setNeedsPaints()",
            &BrowserTab::setNeedsPaint,
            m_activeTab
        )
    );
    clearData();
    m_needsAnimationFrame = true;
    m_animationTimer = nullptr;
}

void Browser::clearData()
{
    m_logger.info("clearing data");
    m_activeTabScroll = 0;
    m_activeTabUrl = std::nullopt;
    m_activeTabDisplayList.clear();
    m_compositedLayers.clear();
    m_compositedUpdates.clear();
    m_accessibilityTree = std::nullopt;
}

void Browser::goBack()
{
    auto task = std::make_unique<Task<BrowserTab*>>(
        "BrowserTab::goBack",
        &BrowserTab::goBack,
        m_activeTab
    );
    m_activeTab->taskRunner.scheduleTask(std::move(task));
    clearData();
}

void Browser::focusAddressBar()
{
    std::lock_guard lock(m_mutex);
    m_chrome->focusAddressBar();
    setNeedsRaster();
}

void Browser::changeAddress(QString newAddress)
{
    std::lock_guard lock(m_mutex);
    if (m_tabs.empty())
    {
        newTab(URL(newAddress.toStdString()));
    }
    else 
    {
        auto url = URL(newAddress.toStdString());
        scheduleLoad(url);
        m_view->setFocus();
        m_chrome->setAddress(newAddress);
    }
}

size_t Browser::getActiveTabIndex()
{
    auto it = std::ranges::find_if(m_tabs,
        [this](BrowserTab* tab) { return tab == m_activeTab; },
        &std::unique_ptr<BrowserTab>::get
    );

    if (it != m_tabs.end())
    {
        auto activeIndex = std::distance(m_tabs.begin(), it);
        return activeIndex;
    }

    return 0;
}

void Browser::cycleTabs()
{
    std::lock_guard lock(m_mutex);
    auto activeIndex = getActiveTabIndex();
    auto newIndex = (activeIndex + 1) % m_tabs.size();
    m_chrome->setActiveTab(newIndex);
    setActiveTab(m_tabs[newIndex].get());
}

void Browser::handleClick(Point point)
{
    std::lock_guard lock(m_mutex);
    if (!m_activeTab->isLoaded()) return;
    m_view->setFocus();
    auto task = std::make_unique<Task<BrowserTab*, Point>>(
        "BrowserTab::click(" + point.describe() + ")",
        &BrowserTab::click,
        m_activeTab,
        point
    );
    m_activeTab->taskRunner.scheduleTask(std::move(task));
}

void Browser::handleKeyPress(std::string text)
{
    std::lock_guard lock(m_mutex);
    auto task = std::make_unique<Task<BrowserTab*, std::string>>(
        "BrowserTab::keyPress(" + text + ")",
        &BrowserTab::keyPress,
        m_activeTab,
        text
    );
    m_activeTab->taskRunner.scheduleTask(std::move(task));
}

void Browser::handleResize(int width, int height)
{
    auto task = std::make_unique<Task<BrowserTab*, int, int>>(
        "BrowserTab::resize(" + std::to_string(width) + ", " + std::to_string(height) + ")", 
        &BrowserTab::resize,
        m_activeTab, 
        width,
        height
    );
    m_activeTab->taskRunner.scheduleTask(std::move(task));
}

int Browser::clampScroll(const int scroll)
{
    int maxScroll = m_activeTabHeight - m_view->size().height();
    return std::max(0, std::min(scroll, maxScroll));
}

void Browser::handleScrollDown()
{
    std::lock_guard lock(m_mutex);
    if (!m_activeTabHeight) return;
    m_activeTabScroll = clampScroll(m_activeTabScroll + Constants::SCROLL_STEP);
    setNeedsRaster();
    m_needsAnimationFrame = true;
}

void Browser::handleScrollUp()
{
    std::lock_guard lock(m_mutex);
    if (!m_activeTabHeight) return;
    m_activeTabScroll = clampScroll(m_activeTabScroll - Constants::SCROLL_STEP);
    setNeedsRaster();
    m_needsAnimationFrame = true;
}

void Browser::handleEnter()
{
    if (m_focus == "content")
    {
        m_activeTab->taskRunner.scheduleTask(
            std::make_unique<Task<BrowserTab*>>(
                "BrowserTab::enter()",
                &BrowserTab::enter,
                m_activeTab
            )
        );
    }
}

void Browser::handleTab()
{
    m_focus = "content";
    m_activeTab->taskRunner.scheduleTask(
        std::make_unique<Task<BrowserTab*>>(
            "BrowserTab::advanceTab(\"" + *m_focus + "\")",
            &BrowserTab::advanceTab,
            m_activeTab
        )
    );
}

void Browser::handleAccessibilityToggle()
{
    std::lock_guard lock(m_mutex);
    m_accessibilityEnabled = !m_accessibilityEnabled;
    setNeedsAccessibility();
}

void Browser::updateAccessibility()
{
    if (!m_accessibilityTree) return;

    if (!m_hasSpokenDocument)
    {
        speakDocument();
        m_hasSpokenDocument = true;
    }

    // If the current tab's focus is not the last tab focus, we
    // know focus has moved and we need to speak the focused node.
    if (m_tabFocus && m_tabFocus != m_lastTabFocus)
    {
        std::vector<AccessibilityNode*> treeList;
        treeToList(m_accessibilityTree->get(), treeList);

        std::vector<AccessibilityNode*> nodes;
        for (const auto& node : treeList)
        {
            if (node->getNode() == m_tabFocus)
            {
                nodes.push_back(node);
            }
        }

        if (!nodes.empty())
        {
            m_focusAllyNode = nodes[0];
            speakNode(m_focusAllyNode, "element focused ");
        }

        m_lastTabFocus = m_tabFocus;
    }
}

void Browser::speakNode(const AccessibilityNode* node, const std::string& text)
{
    auto textToSpeak = text + node->text;
    if (!textToSpeak.empty() && !node->children.empty() && node->children[0]->role == "StaticText")
    {
        textToSpeak += " " + node->children[0]->text;
    }

    if (!textToSpeak.empty())
    {
        speakText(textToSpeak);
    }
}

void Browser::speakDocument()
{
    std::string text = "Here are the document contents: ";

    std::vector<AccessibilityNode*> treeList;
    treeToList(m_accessibilityTree->get(), treeList);

    for (const auto& node : treeList)
    {
        auto newText = node->text;
        if (!newText.empty())
        {
            text += "\n" + newText;
        }
    }

    speakText(text);
}

void Browser::speakText(const std::string& text)
{
    // TODO: Integrate into the operating system's
    // text-to-speech APIs. For now, log it.
    m_logger.info("Speaking text:\n" + text);
}

void Browser::incrementZoom(bool increment)
{
    auto task = std::make_unique<Task<BrowserTab*, bool>>(
        "BrowserTab::zoom(" + std::to_string(increment) + ")",
        &BrowserTab::zoom,
        m_activeTab,
        increment
    );
    m_activeTab->taskRunner.scheduleTask(std::move(task));
}

void Browser::resetZoom()
{
    auto task = std::make_unique<Task<BrowserTab*>>(
        "BrowserTab::resetZoom()",
        &BrowserTab::resetZoom,
        m_activeTab
    );
    m_activeTab->taskRunner.scheduleTask(std::move(task));
}

void Browser::setDarkMode(bool darkMode)
{
    m_darkMode = darkMode;

    m_activeTab->taskRunner.scheduleTask(
        std::make_unique<Task<BrowserTab*, bool>>(
            "BrowserTab::setDarkMode(" + std::to_string(darkMode) + ")",
            &BrowserTab::setDarkMode,
            m_activeTab,
            darkMode
        )
    );
}

// Debugging helpers
void printCommand(const Command* command, int depth = 0)
{
    QString indent(depth * 2, ' ');
    std::cout << indent.toStdString() << command->describe() << std::endl;
    for (const auto& child : command->children)
        printCommand(child.get(), depth + 1);
}

void Browser::printDrawList()
{
    std::cout << "DRAW LIST:" << std::endl;
    for (const auto& command : m_drawList)
        printCommand(command.get());
}
