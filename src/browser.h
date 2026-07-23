#pragma once

#include "browser_view.h"
#include "browser_tab.h"
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
    std::unique_ptr<BrowserView> m_view;
    std::vector<std::unique_ptr<BrowserTab>> m_tabs;
};
