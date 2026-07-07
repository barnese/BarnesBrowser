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
