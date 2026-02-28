#ifndef MEASUREMENTTOOL_H
#define MEASUREMENTTOOL_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QPointF>
#include <QWidget>
#include <QLabel>

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
    
    // 获取信息面板
    QWidget* getInfoPanel() const;
    
    // 更新主题样式
    void updateTheme(bool isDarkTheme);
    
signals:
    void modeChanged(bool enabled);

private:
    void drawMeasurementLine();
    double calculateDistance(const QPointF &p1, const QPointF &p2);
    void createInfoPanel();
    void updateInfoPanel(double distance, const QPointF &start, const QPointF &end);
    
    QGraphicsScene *m_scene;
    QGraphicsView *m_view;
    
    QGraphicsLineItem *m_measureLine;
    QGraphicsTextItem *m_measureText;
    
    QPointF m_measureStart;
    QPointF m_measureEnd;
    
    bool m_isMeasureMode;
    bool m_isMeasureCompleted;
    bool m_isShiftPressed;
    
    // 信息面板
    QWidget *m_infoPanel;
    QLabel *m_distanceLabel;
    QLabel *m_startPosLabel;
    QLabel *m_endPosLabel;
    QLabel *m_deltaLabel;
    
    bool m_isDarkTheme;
};

#endif // MEASUREMENTTOOL_H
