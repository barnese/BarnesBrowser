#pragma once

#include "browser_view.h"
#include "browser_tab.h"
#include "commit_data.h"
#include "chrome/chrome.h"
#include "helpers/measure_time.h"
#include "logger.h"
#include "url.h"

#include <memory>
#include <optional>
#include <QImage>
#include <QTimer>
#include <QWidget>
#include <vector>

/// Main browser which contains the chrome, tab view,
/// and the main animation refresh rate loop, including
/// time measurements for performance tracking.
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
    void scheduleAnimationFrame();
    void setNeedsAnimationFrame(BrowserTab* tab);
    void setNeedsRasterAndDraw();
    void scheduleLoad(URL url);
    void commit(BrowserTab* tab, CommitData& commitData);

    MeasureTime measure;

private:
    void setActiveTab(BrowserTab* tab);
    void handleClick(Point point);
    void handleKeyPress(std::string text);
    void handleResize(int width, int height);
    void handleScrollDown();
    void handleScrollUp();
    void rasterAndDraw();
    void rasterTab();
    void render();
    void draw();
    int clampScroll(const int scroll);
    void newTabInternal(std::optional<URL> url = std::nullopt);
    void mainTimerFired();

    std::mutex m_mutex;
    BrowserView* m_view;
    Chrome* m_chrome;
    QTimer m_timer;
    QTimer* m_animationTimer;
    bool m_needsAnimationFrame;
    bool m_needsRasterAndDraw;
    
    // Tab-specific members    
    std::vector<std::unique_ptr<BrowserTab>> m_tabs;
    BrowserTab* m_activeTab;
    std::optional<URL> m_activeTabUrl;
    int m_activeTabScroll;
    int m_activeTabHeight;
    std::vector<std::unique_ptr<Command>> m_activeTabDisplayList;
    QImage m_tabSurface;
    Logger m_logger;
};
