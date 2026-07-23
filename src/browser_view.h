#pragma once

#include "browser_tab.h"

#include <QGraphicsView>
#include <QKeyEvent>
#include <QWheelEvent>

/// Holds the browser's graphical window and handles user interactions.
class BrowserView : public QGraphicsView
{
    public:
        explicit BrowserView(QWidget* parent = nullptr);
        void setTab(BrowserTab* tab);

    private:
        void resizeEvent(QResizeEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;
        void wheelEvent(QWheelEvent* event) override;

        BrowserTab* m_activeTab; 
};
