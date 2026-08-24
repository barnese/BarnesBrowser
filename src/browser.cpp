#include "browser.h"

#include "commands/draw_composited_layer.h"
#include "commands/paint_command.h"
#include "constants.h"
#include "helpers/thread_registry.h"

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

    connect(&m_timer, &QTimer::timeout, this, &Browser::mainTimerFired);
    m_timer.start(10);

    m_animationTimer = nullptr;
    m_needsAnimationFrame = false;
    m_needsCompositeRasterAndDraw = false;
    m_activeTabHeight = 0;
    m_activeTabScroll = 0;
    m_activeTabDisplayList.clear();
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
        setNeedsRasterAndDraw();
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

void Browser::setNeedsRasterAndDraw()
{
    m_needsCompositeRasterAndDraw = true;
}

void Browser::compositeRasterAndDraw()
{
    std::lock_guard lock(m_mutex);
    if (!m_needsCompositeRasterAndDraw) return;
    m_logger.info("composite rastering and drawing");
    measure.start("compositeRasterAndDraw");
    composite();
    rasterTab();
    paintDrawList();
    draw();
    measure.stop("compositeRasterAndDraw");
    m_needsCompositeRasterAndDraw = false;
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
    context.scroll = m_activeTabScroll;

    for (const auto& item : m_drawList)
    {
        item->execute(context);
    }
}

/// Converts a tree of commands to a vector.
void treeToList(Command* command, std::vector<Command*>& list)
{
    list.push_back(command);

    for (const auto& child : command->children)    
        treeToList(child.get(), list);
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
    std::vector<Command*> paintCommands;

    for (const auto& command : m_activeTabDisplayList)
    {
        treeToList(command.get(), allCommands);
    }

    for (auto const& command : allCommands)
    {
        if (auto paintCommand = dynamic_cast<PaintCommand*>(command))
        {
            paintCommands.push_back(paintCommand);
        }
    }

    for (auto command : paintCommands)
    {
        auto layer = std::make_unique<CompositedLayer>(command);
        m_compositedLayers.push_back(std::move(layer));
    }
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
            if (newEffects.contains(parent))
            {
                newEffects.at(parent)->children.push_back(std::move(current));
                break;
            }
            else 
            {
                auto cloned = parent->clone(std::move(current));
                newEffects[parent] = cloned.get();
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
        address = QString::fromStdString(url.value().describe(false));
        tabName = address;
    }
    else 
    {
        address = "";
        tabName = "New Tab";
    }
    //m_view->centerOn(0, 0);
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

void Browser::setActiveTab(BrowserTab* tab)
{
    m_activeTab = tab;
    m_activeTabScroll = 0;
    m_activeTabUrl = std::nullopt;
    m_needsAnimationFrame = true;
    m_animationTimer = nullptr;
}

void Browser::goBack()
{
    auto task = std::make_unique<Task<BrowserTab*>>(
        "BrowserTab::goBack",
        &BrowserTab::goBack,
        m_activeTab
    );
    m_activeTab->taskRunner.scheduleTask(std::move(task));
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
        //m_view->centerOn(0, 0);
        m_view->setFocus();
        m_chrome->setAddress(newAddress);
    }
}

void Browser::handleClick(Point point)
{
    std::lock_guard lock(m_mutex);
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
    setNeedsRasterAndDraw();
    m_needsAnimationFrame = true;
}

void Browser::handleScrollUp()
{
    std::lock_guard lock(m_mutex);
    if (!m_activeTabHeight) return;
    m_activeTabScroll = clampScroll(m_activeTabScroll - Constants::SCROLL_STEP);
    setNeedsRasterAndDraw();
    m_needsAnimationFrame = true;
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
