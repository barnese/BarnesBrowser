#pragma once

#include "browser_view.h"
#include "browser_tab.h"
#include "chrome.h"
#include "url.h"

#include <memory>
#include <QWidget>
#include <vector>

/// Main browser UI.
class Browser : public QWidget
{
public:
    Browser(QWidget *parent = nullptr);
    void newTab(const URL url);

private:
    BrowserView* m_view;
    Chrome* m_chrome;
    std::vector<std::unique_ptr<BrowserTab>> m_tabs;
};
