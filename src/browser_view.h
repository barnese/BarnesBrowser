#pragma once

#include "point.h"
#include "commands/paint_context.h"

#include <string>
#include <QKeyEvent>
#include <QOpenGLWidget>
#include <QOpenGLTextureBlitter>
#include <QPainter>
#include <QWheelEvent>

/// Holds the browser's graphical window and handles user interactions.
class BrowserView : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit BrowserView(QWidget* parent = nullptr);

signals:
    void paintRequested(PaintContext& context);
    void click(Point point);
    void keyPress(std::string text);
    void resize(int width, int height);
    void scrollDown();
    void scrollUp();
    void zoom(bool increment);
    void resetZoom();
    void setDarkMode(bool darkMode);
    void goBack();
    void focusAddressBar();
    void newTab();
    void cycleTabs();

protected:
    void initializeGL() override;
    void paintGL() override;

private:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void darkModeToggleEvent();

    QOpenGLTextureBlitter m_blitter;
    bool m_darkMode;
};
