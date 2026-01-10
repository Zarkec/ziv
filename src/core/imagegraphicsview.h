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
    void leaveEvent(QEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
};

#endif // IMAGEGRAPHICSVIEW_H
