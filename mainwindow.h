#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QPixmap>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QLabel>
#include <QImageReader>
#include <QFile>
#include <cmath>

// 添加OpenCV头文件
#include <opencv2/opencv.hpp>

// 自定义GraphicsView类，用于处理鼠标事件
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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openImage();
    void zoomIn();
    void zoomOut();
    void fitToWindow();
    void originalSize();
    void rotateLeft();
    void rotateRight();
    void rotate180();
    void toggleMeasureMode();
    void handleMousePress(QPointF scenePos);
    void handleMouseMove(QPointF scenePos);
    void updateCoordinates(QPointF scenePos);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();
    void setupActions();
    void setupConnections();
    void updateScaleInfo();
    void updateSizeInfo();
    void drawMeasurementLine();
    void clearMeasurement();
    double calculateDistance(const QPointF &p1, const QPointF &p2);

    ImageGraphicsView *m_graphicsView;
    QGraphicsScene *m_graphicsScene;
    QGraphicsPixmapItem *m_pixmapItem;
    QPixmap m_originalPixmap;
    QLabel *m_coordinateLabel;
    QLabel *m_scaleLabel;
    QLabel *m_sizeLabel;
    QAction *m_fitToWindowAction;
    bool m_isFitToWindow;
    
    // 测距功能相关成员变量
    QAction *m_measureAction;
    bool m_isMeasureMode;
    bool m_isMeasureCompleted;
    bool m_isShiftPressed;
    QPointF m_measureStart;
    QPointF m_measureEnd;
    QGraphicsLineItem *m_measureLine;
    QGraphicsTextItem *m_measureText;
};

#endif // MAINWINDOW_H
