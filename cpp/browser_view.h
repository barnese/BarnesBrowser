#pragma once

#include "browser_scene.h"
#include <QGraphicsView>
#include <QKeyEvent>

/// Handles user interactions with the browser.
class BrowserView : public QGraphicsView
{
    public:
        explicit BrowserView(BrowserScene* scene, QWidget* parent = nullptr);

    protected:
        void keyPressEvent(QKeyEvent* event) override;

    private:
        BrowserScene* m_scene;
};
