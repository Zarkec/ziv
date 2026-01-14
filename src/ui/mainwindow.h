#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QResizeEvent>

class ImageViewer;
class MeasurementTool;
class ImageGraphicsView;

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
    void flipHorizontal();
    void flipVertical();
    void exportImage();
    void toggleMeasureMode();
    void onPaletteChanged();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();
    void setupActions();
    void setupConnections();
    void updateThemeIcons();
    bool isSystemDarkTheme();

    ImageGraphicsView *m_graphicsView;
    QGraphicsScene *m_graphicsScene;
    QLabel *m_coordinateLabel;
    QLabel *m_scaleLabel;
    QLabel *m_sizeLabel;
    QLabel *m_imageSizeLabel;
    QAction *m_fitToWindowAction;
    QAction *m_measureAction;
    
    ImageViewer *m_imageViewer;
    MeasurementTool *m_measurementTool;
    
    QSlider *m_zoomSlider;
    QSpinBox *m_zoomSpinBox;
    
    bool m_isDarkTheme;
    QMap<QString, QAction*> m_iconActions;
};

#endif // MAINWINDOW_H
