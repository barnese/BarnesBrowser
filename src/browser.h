#pragma once

#include "browser_view.h"
#include "browser_tab.h"
#include "chrome/chrome.h"
#include "helpers/measure_time.h"
#include "url.h"

#include <memory>
#include <optional>
#include <QImage>
#include <QTimer>
#include <QWidget>
#include <vector>

/// Main browser UI.
class Browser : public QWidget
{
    Q_OBJECT

public:
    Browser(QWidget *parent = nullptr);
    ~Browser();
    void newTab(std::optional<URL> url = std::nullopt);
    void createNewTab();
    void changeTab(size_t index);
    void changeAddress(QString newAddress);
    void goBack();
    void linkClicked(URL url);
    void refreshTab(const QImage& surface);
    void scheduleAnimationFrame();
    void setNeedsAnimationFrame(BrowserTab* tab);
    MeasureTime measure;

private:
    BrowserView* m_view;
    Chrome* m_chrome;
    std::vector<std::unique_ptr<BrowserTab>> m_tabs;
    BrowserTab* m_activeTab;
    QTimer m_animationTimer;
    bool m_needsAnimationFrame;
};
