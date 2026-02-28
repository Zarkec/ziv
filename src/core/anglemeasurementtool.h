#ifndef ANGLEMEASUREMENTTOOL_H
#define ANGLEMEASUREMENTTOOL_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsPathItem>
#include <QGraphicsView>
#include <QPointF>
#include <QWidget>
#include <QLabel>

class AngleMeasurementTool : public QObject
{
    Q_OBJECT

public:
    explicit AngleMeasurementTool(QGraphicsScene *scene, QGraphicsView *view, QObject *parent = nullptr);
    
    void toggleAngleMode(bool enabled);
    void handleMousePress(QPointF scenePos);
    void handleMouseMove(QPointF scenePos);
    void setShiftPressed(bool pressed);
    void clearMeasurement();
    void updateMeasurementScale();
    
    bool isAngleMode() const;
    
    // 获取信息面板
    QWidget* getInfoPanel() const;
    
    // 更新主题样式
    void updateTheme(bool isDarkTheme);
    
signals:
    void modeChanged(bool enabled);

private:
    void drawAngleMeasurement();
    double calculateAngle(const QPointF &p1, const QPointF &p2);
    double calculateAngleDifference(double angle1, double angle2);
    QPointF constrainAngle(const QPointF &vertex, const QPointF &point, double targetAngle);
    void createInfoPanel();
    void updateInfoPanel(double angle, const QPointF &vertex);
    
    QGraphicsScene *m_scene;
    QGraphicsView *m_view;
    
    QGraphicsLineItem *m_firstLine;
    QGraphicsLineItem *m_secondLine;
    QGraphicsTextItem *m_angleText;
    QGraphicsPathItem *m_angleArc;
    
    QPointF m_vertex;
    QPointF m_firstEndPoint;
    QPointF m_secondEndPoint;
    
    int m_clickCount;
    bool m_isAngleMode;
    bool m_isAngleCompleted;
    bool m_isShiftPressed;
    
    // 信息面板
    QWidget *m_infoPanel;
    QLabel *m_angleLabel;
    QLabel *m_vertexLabel;
    QLabel *m_line1Label;
    QLabel *m_line2Label;
    
    bool m_isDarkTheme;
};

#endif // ANGLEMEASUREMENTTOOL_H
