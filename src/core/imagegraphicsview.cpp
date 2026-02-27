#include "imagegraphicsview.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QEnterEvent>
#include <QPainter>
#include <QScrollBar>

ImageGraphicsView::ImageGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
    , m_mouseInView(false)
    , m_rightButtonDragging(false)
    , m_isCrosshairMode(false)
    , m_hasFixedCrosshair(false)
{
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setMouseTracking(true);
}

void ImageGraphicsView::mousePressEvent(QMouseEvent *event)
{
    // 在 CrossCursor 模式下（测距/测角模式），右键用于拖动
    if (event->button() == Qt::RightButton && cursor().shape() == Qt::CrossCursor) {
        m_rightButtonDragging = true;
        m_isCrosshairMode = true;
        m_dragStartPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    
    QGraphicsView::mousePressEvent(event);
    
    QPointF viewPos = event->pos();
    QPointF scenePos = mapToScene(viewPos.toPoint());
    emit mousePressed(scenePos);
}

void ImageGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    m_lastMousePos = event->pos();
    
    // 处理右键拖动
    if (m_rightButtonDragging) {
        QPoint delta = event->pos() - m_dragStartPos;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        m_dragStartPos = event->pos();
        viewport()->update();
        event->accept();
        return;
    }
    
    QGraphicsView::mouseMoveEvent(event);
    
    if (cursor().shape() == Qt::CrossCursor) {
        viewport()->update();
    }
    
    QPointF viewPos = event->pos();
    QPointF scenePos = mapToScene(viewPos.toPoint());
    emit mouseMoved(scenePos);
}

void ImageGraphicsView::leaveEvent(QEvent *event)
{
    QGraphicsView::leaveEvent(event);
    m_mouseInView = false;
    viewport()->update();
    emit mouseLeft();
}

void ImageGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    // 处理右键拖动结束
    if (event->button() == Qt::RightButton && m_rightButtonDragging) {
        m_rightButtonDragging = false;
        setCursor(Qt::CrossCursor);
        m_isCrosshairMode = false;
        event->accept();
        return;
    }
    
    QGraphicsView::mouseReleaseEvent(event);
}

void ImageGraphicsView::enterEvent(QEnterEvent *event)
{
    QGraphicsView::enterEvent(event);
    
    m_mouseInView = true;
    m_lastMousePos = event->position().toPoint();
    
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

void ImageGraphicsView::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);
    
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_hasFixedCrosshair) {
        QPoint fixedPos = mapFromScene(m_fixedCrosshairPosition);
        QPen fixedPen(QColor(255, 255, 0, 200));
        fixedPen.setWidth(3);
        fixedPen.setStyle(Qt::SolidLine);
        painter.setPen(fixedPen);

        painter.drawLine(0, fixedPos.y(), viewport()->width(), fixedPos.y());
        painter.drawLine(fixedPos.x(), 0, fixedPos.x(), viewport()->height());
    }

    if ((cursor().shape() == Qt::CrossCursor || m_isCrosshairMode) && !m_hasFixedCrosshair && m_mouseInView) {
        QPen pen(QColor(0, 255, 0, 200));
        pen.setWidth(2);
        pen.setStyle(Qt::DashLine);
        painter.setPen(pen);

        painter.drawLine(0, m_lastMousePos.y(), viewport()->width(), m_lastMousePos.y());
        painter.drawLine(m_lastMousePos.x(), 0, m_lastMousePos.x(), viewport()->height());
    }
}

void ImageGraphicsView::setFixedCrosshairPosition(const QPointF &scenePos)
{
    m_fixedCrosshairPosition = scenePos;
    m_hasFixedCrosshair = true;
    viewport()->update();
}

void ImageGraphicsView::clearFixedCrosshair()
{
    m_hasFixedCrosshair = false;
    viewport()->update();
}
