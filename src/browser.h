#pragma once

#include "browser_view.h"
#include "browser_tab.h"
#include "chrome/chrome.h"
#include "url.h"

#include <memory>
#include <optional>
#include <QWidget>
#include <vector>

/// Main browser UI.
class Browser : public QWidget
{
    Q_OBJECT

public:
    Browser(QWidget *parent = nullptr);
    void newTab(std::optional<URL> url = std::nullopt);
    void createNewTab();
    void changeTab(size_t index);

private:
    BrowserView* m_view;
    Chrome* m_chrome;
    std::vector<std::unique_ptr<BrowserTab>> m_tabs;
};
