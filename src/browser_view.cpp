#include "browser_view.h"

#include <QOpenGLWidget>

BrowserView::BrowserView(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void BrowserView::initializeGL()
{
    m_blitter.create();
}

void BrowserView::paintGL()
{
    QPainter painter(this);

    painter.fillRect(rect(), Qt::white);

    PaintContext context {
        painter,
        &m_blitter,
        size()
    };

    emit paintRequested(context);
}

void BrowserView::resizeEvent(QResizeEvent* event)
{
    QOpenGLWidget::resizeEvent(event);
    int width = size().width();
    int height = size().height();
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
        QOpenGLWidget::keyPressEvent(event);
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
        QOpenGLWidget::mousePressEvent(event);
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
        QOpenGLWidget::wheelEvent(event);
    }
}
