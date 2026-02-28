#ifndef BRUSHTOOL_H
#define BRUSHTOOL_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPointF>
#include <QColor>
#include <QWidget>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QStack>

class QGraphicsPathItem;
class QPainterPath;

class BrushTool : public QObject
{
    Q_OBJECT

public:
    explicit BrushTool(QGraphicsScene *scene, QGraphicsView *view, QObject *parent = nullptr);
    ~BrushTool();

    void toggleBrushMode(bool enabled);
    void handleMousePress(QPointF scenePos);
    void handleMouseMove(QPointF scenePos);
    void handleMouseRelease(QPointF scenePos);
    
    bool isBrushMode() const;
    
    QWidget* getInfoPanel() const;
    
    void updateTheme(bool isDarkTheme);
    
    void clearBrushStrokes();
    
    void setBrushColor(const QColor &color);
    QColor brushColor() const;
    
    void setBrushSize(int size);
    int brushSize() const;
    
    void setBrushOpacity(int opacity);
    int brushOpacity() const;
    
    void onScaleChanged();
    
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;

signals:
    void modeChanged(bool enabled);

private:
    void createInfoPanel();
    void updateBrushPreview();

private:
    QGraphicsScene *m_scene;
    QGraphicsView *m_view;
    
    bool m_isBrushMode;
    bool m_isDrawing;
    
    QPointF m_lastPoint;
    
    QList<QGraphicsPathItem*> m_brushStrokes;
    QStack<QGraphicsPathItem*> m_undoStack;
    QStack<QGraphicsPathItem*> m_redoStack;
    QGraphicsPathItem *m_currentPath;
    QPainterPath *m_currentPainterPath;
    
    QColor m_brushColor;
    int m_brushSize;
    int m_brushOpacity;
    
    QWidget *m_infoPanel;
    QLabel *m_colorPreview;
    QPushButton *m_colorButton;
    QSlider *m_sizeSlider;
    QSpinBox *m_sizeSpinBox;
    QSlider *m_opacitySlider;
    QSpinBox *m_opacitySpinBox;
    
    bool m_isDarkTheme;
};

#endif
