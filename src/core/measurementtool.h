#ifndef MEASUREMENTTOOL_H
#define MEASUREMENTTOOL_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QPointF>

class MeasurementTool : public QObject
{
    Q_OBJECT

public:
    explicit MeasurementTool(QGraphicsScene *scene, QGraphicsView *view, QObject *parent = nullptr);
    
    void toggleMeasureMode(bool enabled);
    void handleMousePress(QPointF scenePos);
    void handleMouseMove(QPointF scenePos);
    void setShiftPressed(bool pressed);
    void clearMeasurement();
    void updateMeasurementScale();
    
    bool isMeasureMode() const;
    
signals:
    void modeChanged(bool enabled);

private:
    void drawMeasurementLine();
    double calculateDistance(const QPointF &p1, const QPointF &p2);
    
    QGraphicsScene *m_scene;
    QGraphicsView *m_view;
    
    QGraphicsLineItem *m_measureLine;
    QGraphicsTextItem *m_measureText;
    
    QPointF m_measureStart;
    QPointF m_measureEnd;
    
    bool m_isMeasureMode;
    bool m_isMeasureCompleted;
    bool m_isShiftPressed;
};

#endif // MEASUREMENTTOOL_H
