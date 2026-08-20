#include "browser.h"

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
    
    connect(m_view, &BrowserView::click, this, &Browser::handleClick);
    connect(m_view, &BrowserView::keyPress, this, &Browser::handleKeyPress);
    connect(m_view, &BrowserView::resize, this, &Browser::handleResize);
    connect(m_view, &BrowserView::scrollDown, this, &Browser::handleScrollDown);
    connect(m_view, &BrowserView::scrollUp, this, &Browser::handleScrollUp);

    connect(&m_timer, &QTimer::timeout, this, &Browser::mainTimerFired);
    m_timer.start(10);

    m_animationTimer = nullptr;
    m_needsAnimationFrame = false;
    m_needsRasterAndDraw = false;
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
    rasterAndDraw();
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
    m_needsRasterAndDraw = true;
}

void Browser::rasterAndDraw()
{
    std::lock_guard lock(m_mutex);
    if (!m_needsRasterAndDraw) return;
    m_logger.info("rastering and drawing");
    measure.start("rasterAndDraw");
    rasterTab();
    draw();
    measure.stop("rasterAndDraw");
    m_needsRasterAndDraw = false;
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
    if (m_activeTabHeight == 0)
    {
        return;
    }

    if (m_activeTabHeight < m_view->size().height())
    {
        m_activeTabHeight = m_view->size().height();
    }

    if (m_tabSurface.isNull() || m_tabSurface.height() != m_activeTabHeight)
    {
        m_tabSurface = QImage(QSize(m_view->size().width(), m_activeTabHeight), QImage::Format_RGBA8888);
    }

    m_tabSurface.fill(Qt::white);

    for (const auto& command : m_activeTabDisplayList)
    {
        command->execute(&m_tabSurface);
    }
}

void Browser::draw()
{
    auto tabRect = QRect(0, m_activeTabScroll, m_view->size().width(), m_activeTabHeight);
    auto visibleSurface = m_tabSurface.copy(tabRect);
    m_view->updateSurface(visibleSurface);
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
    m_view->centerOn(0, 0);
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
        m_view->centerOn(0, 0);
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
