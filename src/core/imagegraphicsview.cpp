#include "imagegraphicsview.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QEnterEvent>
#include <QPainter>

ImageGraphicsView::ImageGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void ImageGraphicsView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    
    QPointF viewPos = event->pos();
    QPointF scenePos = mapToScene(viewPos.toPoint());
    emit mousePressed(scenePos);
}

void ImageGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    
    QPointF viewPos = event->pos();
    QPointF scenePos = mapToScene(viewPos.toPoint());
    emit mouseMoved(scenePos);
}

void ImageGraphicsView::leaveEvent(QEvent *event)
{
    QGraphicsView::leaveEvent(event);
    emit mouseLeft();
}

void ImageGraphicsView::enterEvent(QEnterEvent *event)
{
    QGraphicsView::enterEvent(event);
    
    QPointF viewPos = event->position();
    QPointF scenePos = mapToScene(viewPos.toPoint());
    emit mouseEntered(scenePos);
}

void ImageGraphicsView::wheelEvent(QWheelEvent *event)
{
    qreal scaleFactor = 1.15;
    if (event->angleDelta().y() < 0) {
        scaleFactor = 1.0 / scaleFactor;
    }
    
    qreal currentScale = transform().m11() * 100;
    qreal newScale = currentScale * scaleFactor;
    
    if ((scaleFactor > 1 && currentScale < 3200) || (scaleFactor < 1 && currentScale > 1)) {
        qreal actualScaleFactor = scaleFactor;
        if (scaleFactor > 1 && currentScale < 3200) {
            actualScaleFactor = 3200 / currentScale;
            actualScaleFactor = qMin(actualScaleFactor, 1.15);
        } else if (scaleFactor < 1 && currentScale > 1) {
            actualScaleFactor = 1.0 / 1.15;
            if (currentScale * actualScaleFactor < 1) {
                actualScaleFactor = 1.0 / currentScale;
            }
        }
        
        scale(actualScaleFactor, actualScaleFactor);
        emit scaleChanged();
        event->accept();
    }
}

void ImageGraphicsView::keyPressEvent(QKeyEvent *event)
{
    QGraphicsView::keyPressEvent(event);
    
    if (event->key() == Qt::Key_Shift) {
        emit shiftPressed();
    }
}

void ImageGraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    QGraphicsView::keyReleaseEvent(event);
    
    if (event->key() == Qt::Key_Shift) {
        emit shiftReleased();
    }
}
