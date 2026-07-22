#include "browser_view.h"

BrowserView::BrowserView(BrowserScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent),
      m_scene(scene)
{
}

void BrowserView::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Down:
        m_scene->scrollDown();
        break;        

    case Qt::Key_Up:
        m_scene->scrollUp();        
        break;

    default:
        QGraphicsView::keyPressEvent(event);
    }
}

void BrowserView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        auto point = Point(event->pos().x(), event->pos().y());
        m_scene->click(point);
    }
    else 
    {
        QGraphicsView::mousePressEvent(event);
    }
}

void BrowserView::wheelEvent(QWheelEvent* event)
{
    auto deltaY = event->angleDelta().y();

    if (deltaY > 0)
    {
        m_scene->scrollUp();
    }
    else if (deltaY < 0)
    {
        m_scene->scrollDown();
    }
    else
    {
        QGraphicsView::wheelEvent(event);
    }
}
