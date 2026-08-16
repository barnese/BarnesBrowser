#include "browser.h"

#include "browser_tab.h"
#include "constants.h"

#include <QBoxLayout>

Browser::Browser(QWidget* parent) : QWidget(parent)
{
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
    
    // Sets up a timer to handle animation frames per the refresh rate.
    connect(&m_animationTimer, &QTimer::timeout, this, &Browser::scheduleAnimationFrame);
    m_animationTimer.start(Constants::REFRESH_RATE_MS);
    m_needsAnimationFrame = true;
}

void Browser::scheduleAnimationFrame()
{
    if (m_needsAnimationFrame)
    {
        m_needsAnimationFrame = false;
        auto task = std::make_unique<Task<BrowserTab*>>("", &BrowserTab::runAnimationFrame, m_activeTab);
        m_activeTab->scheduleTask(std::move(task));
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
    int width = size().width();
    int height = size().height();
    auto tab = std::make_unique<BrowserTab>(width, height);
    QString tabName;
    QString address;
    if (url.has_value())
    {
        tab->load(*url);
        address = QString::fromStdString((*url).describe(false));
        tabName = address;
    }
    else 
    {
        address = "";
        tabName = "New Tab";
    }
    m_tabs.push_back(std::move(tab));
    m_activeTab = m_tabs.back().get();
    connect(m_activeTab, &BrowserTab::linkClicked, this, &Browser::linkClicked);
    connect(m_activeTab, &BrowserTab::needsDisplayUpdate, this, &Browser::refreshTab);
    connect(m_activeTab, &BrowserTab::needsAnimationFrame, this, &Browser::setNeedsAnimationFrame);
    m_view->setTab(m_activeTab);
    m_view->centerOn(0, 0);
    m_view->setFocus();
    m_chrome->addTab(tabName, address, m_tabs.size() - 1);
    m_activeTab->didChangeState(true);
}

void Browser::linkClicked(URL url)
{
    QString urlString = QString::fromStdString(url.describe(false));
    m_chrome->setAddress(urlString);
}

void Browser::refreshTab(const QImage& surface)
{
    m_view->refreshTab(surface);
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

    // Mark the current tab as inactive.
    m_activeTab->didChangeState(false);

    const auto& tab = m_tabs.at(index);
    m_view->setTab(tab.get());
    m_activeTab = tab.get();
    
    // Mark the new tab as active.
    m_activeTab->didChangeState(true);
}

void Browser::goBack()
{
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
        m_activeTab->load(newAddress.toStdString());
        m_view->centerOn(0, 0);
        m_view->setFocus();
        m_chrome->setAddress(newAddress);
    }
}
