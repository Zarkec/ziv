#ifndef IMAGEGRAPHICSVIEW_H
#define IMAGEGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>

class ImageGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageGraphicsView(QWidget *parent = nullptr);

    void setFixedCrosshairPosition(const QPointF &scenePos);
    void clearFixedCrosshair();

signals:
    void mouseMoved(QPointF scenePos);
    void mousePressed(QPointF scenePos);
    void mouseLeft();
    void mouseEntered(QPointF scenePos);
    void scaleChanged();
    void shiftPressed();
    void shiftReleased();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QPoint m_lastMousePos;
    bool m_mouseInView;
    bool m_rightButtonDragging;
    QPoint m_dragStartPos;
    bool m_isCrosshairMode;
    QPointF m_fixedCrosshairPosition;
    bool m_hasFixedCrosshair;
};

#endif // IMAGEGRAPHICSVIEW_H
