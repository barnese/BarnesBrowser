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
}

void Browser::newTab(const URL url)
{
    int width = size().width();
    int height = size().height();
    auto tab = std::make_unique<BrowserTab>(width, height);
    tab->load(url);
    m_tabs.push_back(std::move(tab));

    m_view->setTab(m_tabs.back().get());
    m_view->centerOn(0, 0);
}
