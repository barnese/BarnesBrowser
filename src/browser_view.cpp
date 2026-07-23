#include "browser_view.h"

BrowserView::BrowserView(QWidget* parent)
    : QGraphicsView(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void BrowserView::setTab(BrowserTab* tab)
{
    m_activeTab = tab;
    setScene(m_activeTab);
}

void BrowserView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    if (m_activeTab)
    {
        int width = viewport()->size().width();
        int height = viewport()->size().height();
        m_activeTab->resize(width, height);
    }
}

void BrowserView::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Down:
        m_activeTab->scrollDown();
        break;        

    case Qt::Key_Up:
        m_activeTab->scrollUp();        
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
        m_activeTab->click(point);
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
        m_activeTab->scrollUp();
    }
    else if (deltaY < 0)
    {
        m_activeTab->scrollDown();
    }
    else
    {
        QGraphicsView::wheelEvent(event);
    }
}
