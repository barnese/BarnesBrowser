#include "browser.h"
#include "browser_tab.h"
#include "browser_view.h"

#include <QBoxLayout>
#include <memory>

Browser::Browser(QWidget* parent) : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    m_view = std::make_unique<BrowserView>(this);
    layout->addWidget(m_view.get());
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
