#pragma once

#include "point.h"

#include <string>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QWheelEvent>


/// Holds the browser's graphical window and handles user interactions.
class BrowserView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit BrowserView(QWidget* parent = nullptr);
    void updateSurface(const QImage& surface);

signals:
    void click(Point point);
    void keyPress(std::string text);
    void resize(int width, int height);
    void scrollDown();
    void scrollUp();

private:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    QGraphicsScene m_scene;
    QGraphicsPixmapItem* m_pixmapItem;
};
