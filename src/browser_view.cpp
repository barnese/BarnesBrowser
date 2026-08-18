#include "browser_view.h"

#include <QPixmap>

BrowserView::BrowserView(QWidget* parent)
    : QGraphicsView(parent),
      m_scene{this},
      m_pixmapItem(m_scene.addPixmap({}))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setScene(&m_scene);
}

void BrowserView::updateSurface(const QImage& surface)
{
    // Translate the given QImage surface into a QPixmap,
    // which creates a display-optimized version of the surface.
    // PixmapItem then displays the Pixmap onto the scene.
    m_pixmapItem->setPixmap(QPixmap::fromImage(surface));
    m_scene.setSceneRect(m_pixmapItem->boundingRect());
}

void BrowserView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    int width = viewport()->size().width();
    int height = viewport()->size().height();
    resize(width, height);
}

void BrowserView::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Down:
        scrollDown();
        break;        

    case Qt::Key_Up:
        scrollUp();        
        break;

    default:
        auto eventText = event->text().toStdString();
        if (!eventText.empty())
        {
            keyPress(eventText);
        }
        QGraphicsView::keyPressEvent(event);
    }
}

void BrowserView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        auto point = Point(event->pos().x(), event->pos().y());
        click(point);
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
        scrollUp();
    }
    else if (deltaY < 0)
    {
        scrollDown();
    }
    else
    {
        QGraphicsView::wheelEvent(event);
    }
}
