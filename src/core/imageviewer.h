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
#include <QTimer>
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

    // Overlay mode functions
    void enableOverlayMode(bool enable);
    bool isOverlayMode() const;

    bool loadSecondImage(const QString &fileName);
    void clearSecondImage();

    void setAlpha1(double alpha);
    void setAlpha2(double alpha);
    double getAlpha1() const;
    double getAlpha2() const;

    bool exportOverlayImage(const QString &fileName);
    QFuture<bool> exportOverlayImageAsync(const QString &fileName);

signals:
    void imageLoaded(const QString &fileName);
    void scaleChanged();
    void fitToWindowChanged(bool fit);
    void imageIndexChanged(int currentIndex, int totalCount);
    void imageLoadingStarted();
    void imageLoadingFinished();
    void overlayModeChanged(bool enabled);
    void secondImageLoaded(const QString &fileName);
    void secondImageCleared();

private:
    void updateSizeInfo();
    void updatePixmapFromMat();
    void loadImagesFromDirectory(const QString &directoryPath);
    void saveCurrentPosition();
    void loadSavedPosition();
    void updateImageIndexLabel();

    // Overlay helper functions
    void alignImages(const cv::Mat &img1, const cv::Mat &img2,
                     cv::Mat &aligned1, cv::Mat &aligned2);
    cv::Mat computeOverlay();
    void updateOverlay();

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

    // Overlay mode members
    cv::Mat m_cvImage2;
    QPixmap m_originalPixmap2;
    QString m_currentImage2Path;

    bool m_isOverlayMode;
    double m_alpha1;
    double m_alpha2;

    cv::Mat m_overlayResult;
    QTimer *m_overlayUpdateTimer;
};

#endif // IMAGEVIEWER_H
