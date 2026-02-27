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
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QPushButton>

class ImageViewer;
class MeasurementTool;
class AngleMeasurementTool;
class ColorPickerTool;
class ImageGraphicsView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    void openFile(const QString &fileName);

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
    void toggleAngleMode();
    void toggleColorPickerMode();
    void nextImage();
    void previousImage();
    void onPaletteChanged();
    void toggleOverlayMode();
    void loadSecondImage();
    void clearSecondImage();
void onAlpha1Changed(int value);
    void onAlpha2Changed(int value);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void setupUI();
    void setupActions();
    void setupConnections();
    void updateThemeIcons();
    bool isSystemDarkTheme();
    void createOverlayControlPanel();

    ImageGraphicsView *m_graphicsView;
    QGraphicsScene *m_graphicsScene;
    QLabel *m_coordinateLabel;
    QLabel *m_scaleLabel;
    QLabel *m_sizeLabel;
    QLabel *m_imageSizeLabel;
    QLabel *m_imageIndexLabel;
    QLabel *m_loadingLabel;
    QAction *m_fitToWindowAction;
    QAction *m_measureAction;
    QAction *m_angleAction;
    QAction *m_colorPickerAction;

    ImageViewer *m_imageViewer;
    MeasurementTool *m_measurementTool;
    AngleMeasurementTool *m_angleMeasurementTool;
    ColorPickerTool *m_colorPickerTool;

    QSlider *m_zoomSlider;
    QSpinBox *m_zoomSpinBox;

    bool m_isDarkTheme;
    QMap<QString, QAction*> m_iconActions;

    // Overlay mode UI components
    QAction *m_overlayModeAction;
    QWidget *m_overlayControlPanel;
    QWidget *m_rightPanel;  // 右侧颜色信息面板区域

    QLabel *m_image2PathLabel;
    QPushButton *m_loadImage2Button;
    QPushButton *m_clearImage2Button;

    QLabel *m_alpha1Label;
    QSlider *m_alpha1Slider;
    QSpinBox *m_alpha1SpinBox;

    QLabel *m_alpha2Label;
    QSlider *m_alpha2Slider;
    QSpinBox *m_alpha2SpinBox;
};

#endif // MAINWINDOW_H
