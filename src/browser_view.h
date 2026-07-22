#pragma once

#include "browser_scene.h"

#include <QGraphicsView>
#include <QKeyEvent>
#include <QWheelEvent>

/// Handles user interactions with the browser.
class BrowserView : public QGraphicsView
{
    public:
        explicit BrowserView(BrowserScene* scene, QWidget* parent = nullptr);

    private:
        void keyPressEvent(QKeyEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;
        void wheelEvent(QWheelEvent* event) override;

        BrowserScene* m_scene;
};
