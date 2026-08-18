#include "browser.h"

#include "browser_tab.h"
#include "constants.h"

#include <QBoxLayout>
#include <iostream>

Browser::Browser(QWidget* parent) : QWidget(parent)
{
    std::cout << "[Browser] Starting. Thread ID = " << std::this_thread::get_id() << std::endl;

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

    // Sets up a timer to handle animation frames per the refresh rate.
    connect(&m_animationTimer, &QTimer::timeout, this, &Browser::animationFrameRefresh);
    m_animationTimer.start(Constants::REFRESH_RATE_MS);
    m_needsAnimationFrame = true;
}

Browser::~Browser()
{
    measure.finish();
    for (const auto& tab : m_tabs)
    {
        tab->taskRunner.setNeedsQuit();
    }
}

void Browser::animationFrameRefresh()
{
    if (m_needsAnimationFrame)
    {
        m_needsAnimationFrame = false;
        m_activeTab->runAnimationFrame();
    }
}

void Browser::setNeedsAnimationFrame(BrowserTab* tab)
{
    if (tab == m_activeTab)
    {
        m_needsAnimationFrame = true;
    }
}

void Browser::newTab(std::optional<URL> url)
{
    auto tab = std::make_unique<BrowserTab>(size().width(), size().height());
    tab->browser = this;
    m_tabs.push_back(std::move(tab));
    m_activeTab = m_tabs.back().get();

    QString tabName;
    QString address;
    if (url.has_value())
    {
        scheduleLoad(*url);
        address = QString::fromStdString((*url).describe(false));
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

void Browser::linkClicked(URL url)
{
    QString urlString = QString::fromStdString(url.describe(false));
    m_chrome->setAddress(urlString);
}

void Browser::updateSurface(const QImage& surface)
{
    m_view->updateSurface(surface);
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

    setActiveTab(m_tabs.at(index).get());
}

void Browser::setActiveTab(BrowserTab* tab)
{
    m_view->updateSurface(*tab->getSurface());
    m_activeTab = tab;
}

void Browser::goBack()
{
    // TODO: schedule a task for this
    auto url = m_activeTab->goBack();
    if (url.has_value())
    {
        QString urlString = QString::fromStdString(url.value().describe(false));
        m_chrome->setAddress(urlString);
    }
}

void Browser::changeAddress(QString newAddress)
{
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
    // TODO: schedule a task for this
    m_activeTab->click(point);
}

void Browser::handleKeyPress(std::string text)
{
    // TODO: schedule a task for this
    m_activeTab->keyPress(text);
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

void Browser::handleScrollDown()
{
    // TODO: schedule a task for this
    m_activeTab->scrollDown();
}

void Browser::handleScrollUp()
{
    // TODO: schedule a task for this
    m_activeTab->scrollDown();
}
