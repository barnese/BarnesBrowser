#include "browser.h"

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
    m_view->setTab(m_tabs.back().get());
    m_view->centerOn(0, 0);
    m_view->setFocus();
    m_chrome->addTab(tabName, address, m_tabs.size() - 1);
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

    const auto& tab = m_tabs.at(index);
    m_view->setTab(tab.get());
}
