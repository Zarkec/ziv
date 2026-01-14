#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QObject>
#include <QPixmap>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QFuture>
#include <QSettings>
#include <QStringList>
#include <opencv2/opencv.hpp>

#include "core/imagegraphicsview.h"

class ImageViewer : public QObject
{
    Q_OBJECT

public:
    explicit ImageViewer(ImageGraphicsView *view, QGraphicsScene *scene, QObject *parent = nullptr);
    
    void setCoordinateLabel(QLabel *label);
    void setScaleLabel(QLabel *label);
    void setSizeLabel(QLabel *label);
    void setImageSizeLabel(QLabel *label);
    void setZoomSlider(QSlider *slider);
    void setZoomSpinBox(QSpinBox *spinBox);
    void setImageIndexLabel(QLabel *label);
    
    void openImage(const QString &fileName);
    void zoomIn();
    void zoomOut();
    void fitToWindow();
    void originalSize();
    void rotateLeft();
    void rotateRight();
    void rotate180();
    void flipHorizontal();
    void flipVertical();
    bool exportImage(const QString &fileName);
    QFuture<bool> exportImageAsync(const QString &fileName);
    void applyZoom(int percent);
    void updateCoordinates(QPointF scenePos);
    
    void nextImage();
    void previousImage();
    
    bool isEnabled() const;
    QPixmap originalPixmap() const;
    QGraphicsPixmapItem* pixmapItem() const;
    
    void setFitToWindow(bool fit);
    bool isFitToWindow() const;
    
    void resizeEvent();
    void updateScaleInfo();

signals:
    void imageLoaded(const QString &fileName);
    void scaleChanged();
    void fitToWindowChanged(bool fit);
    void imageIndexChanged(int currentIndex, int totalCount);
    void imageLoadingStarted();
    void imageLoadingFinished();

private:
    void updateSizeInfo();
    void updatePixmapFromMat();
    void loadImagesFromDirectory(const QString &directoryPath);
    void saveCurrentPosition();
    void loadSavedPosition();
    void updateImageIndexLabel();
    
    ImageGraphicsView *m_view;
    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_pixmapItem;
    QPixmap m_originalPixmap;
    cv::Mat m_cvImage;
    
    QLabel *m_coordinateLabel;
    QLabel *m_scaleLabel;
    QLabel *m_sizeLabel;
    QLabel *m_imageSizeLabel;
    QLabel *m_imageIndexLabel;
    QSlider *m_zoomSlider;
    QSpinBox *m_zoomSpinBox;
    
    bool m_isFitToWindow;
    qint64 m_fileSize;
    
    QStringList m_imageList;
    int m_currentImageIndex;
    QString m_currentDirectory;
    QSettings *m_settings;
};

#endif // IMAGEVIEWER_H
