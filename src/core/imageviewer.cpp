#include "imageviewer.h"
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QtConcurrent>
#include <opencv2/opencv.hpp>
#include "core/imagegraphicsview.h"

ImageViewer::ImageViewer(ImageGraphicsView *view, QGraphicsScene *scene, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_scene(scene)
    , m_pixmapItem(nullptr)
    , m_coordinateLabel(nullptr)
    , m_scaleLabel(nullptr)
    , m_sizeLabel(nullptr)
    , m_imageSizeLabel(nullptr)
    , m_imageIndexLabel(nullptr)
    , m_zoomSlider(nullptr)
    , m_zoomSpinBox(nullptr)
    , m_isFitToWindow(false)
    , m_fileSize(0)
    , m_currentImageIndex(-1)
    , m_settings(new QSettings("ZivImageViewer", "ImageViewer", this))
    , m_isOverlayMode(false)
    , m_alpha1(0.5)
    , m_alpha2(0.5)
    , m_overlayUpdateTimer(new QTimer(this))
{
    m_overlayUpdateTimer->setSingleShot(true);
    connect(m_overlayUpdateTimer, &QTimer::timeout, this, &ImageViewer::updateOverlay);
}

void ImageViewer::setCoordinateLabel(QLabel *label)
{
    m_coordinateLabel = label;
}

void ImageViewer::setScaleLabel(QLabel *label)
{
    m_scaleLabel = label;
}

void ImageViewer::setSizeLabel(QLabel *label)
{
    m_sizeLabel = label;
}

void ImageViewer::setImageSizeLabel(QLabel *label)
{
    m_imageSizeLabel = label;
}

void ImageViewer::setZoomSlider(QSlider *slider)
{
    m_zoomSlider = slider;
}

void ImageViewer::setZoomSpinBox(QSpinBox *spinBox)
{
    m_zoomSpinBox = spinBox;
}

void ImageViewer::setImageIndexLabel(QLabel *label)
{
    m_imageIndexLabel = label;
}

void ImageViewer::openImage(const QString &fileName)
{
    if (fileName.isEmpty()) {
        return;
    }
    
    emit imageLoadingStarted();
    
    QFileInfo fileInfo(fileName);
    QString directoryPath = fileInfo.absolutePath();
    
    if (m_currentDirectory != directoryPath) {
        m_currentDirectory = directoryPath;
        loadImagesFromDirectory(directoryPath);
        loadSavedPosition();
    }
    
    m_currentImageIndex = m_imageList.indexOf(fileName);
    if (m_currentImageIndex < 0) {
        m_currentImageIndex = 0;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(nullptr, tr("错误"), tr("无法打开图片文件: %1").arg(fileName));
        emit imageLoadingFinished();
        return;
    }
    
    m_fileSize = file.size();
    QByteArray fileData = file.readAll();
    file.close();
    
    cv::Mat matData(1, fileData.size(), CV_8U, (void*)fileData.data());
    
    cv::Mat cvImage = cv::imdecode(matData, cv::IMREAD_UNCHANGED);
    if (cvImage.empty()) {
        QMessageBox::warning(nullptr, tr("错误"), tr("无法解码图片文件: %1").arg(fileName));
        emit imageLoadingFinished();
        return;
    }
    
    m_cvImage = cvImage;
    updatePixmapFromMat();
    
    if (m_pixmapItem) {
        m_scene->removeItem(m_pixmapItem);
        delete m_pixmapItem;
        m_pixmapItem = nullptr;
    }
    
    m_pixmapItem = m_scene->addPixmap(m_originalPixmap);
    m_scene->setSceneRect(m_originalPixmap.rect());
    
    m_view->setEnabled(true);
    if (m_zoomSlider) {
        m_zoomSlider->setEnabled(true);
    }
    if (m_zoomSpinBox) {
        m_zoomSpinBox->setEnabled(true);
    }
    
    m_isFitToWindow = true;
    m_view->fitInView(m_pixmapItem, Qt::KeepAspectRatio);
    
    updateSizeInfo();
    updateScaleInfo();
    updateImageIndexLabel();
    
    emit imageLoaded(fileName);
    emit imageIndexChanged(m_currentImageIndex + 1, m_imageList.size());
    emit imageLoadingFinished();
}

void ImageViewer::zoomIn()
{
    if (!m_view->isEnabled()) {
        return;
    }
    
    qreal currentScale = m_view->transform().m11() * 100;

    if (currentScale < 3200) {
        qreal maxScaleFactor = 1.2;
        if (currentScale * 1.2 > 3200) {
            maxScaleFactor = 3200 / currentScale;
        }
        m_view->scale(maxScaleFactor, maxScaleFactor);
        updateScaleInfo();
        
        if (m_isFitToWindow) {
            m_isFitToWindow = false;
        }
        
        emit scaleChanged();
    }
}

void ImageViewer::zoomOut()
{
    if (!m_view->isEnabled()) {
        return;
    }
    
    qreal currentScale = m_view->transform().m11() * 100;
    
    if (currentScale > 1) {
        m_view->scale(1.0 / 1.2, 1.0 / 1.2);
        updateScaleInfo();
        
        if (m_isFitToWindow) {
            m_isFitToWindow = false;
        }
        
        emit scaleChanged();
    }
}

void ImageViewer::fitToWindow()
{
    if (!m_view->isEnabled() || !m_pixmapItem) {
        return;
    }
    
    if (m_isFitToWindow) {
        m_view->fitInView(m_pixmapItem, Qt::KeepAspectRatio);
    } else {
        originalSize();
    }
    
    updateScaleInfo();
}

void ImageViewer::originalSize()
{
    if (!m_view->isEnabled()) {
        return;
    }
    
    m_view->resetTransform();
    updateScaleInfo();
    
    m_isFitToWindow = false;
    emit fitToWindowChanged(false);
}

void ImageViewer::rotateLeft()
{
    if (!m_view->isEnabled() || m_cvImage.empty()) {
        return;
    }

    cv::Mat rotated;
    cv::transpose(m_cvImage, rotated);
    cv::flip(rotated, rotated, 0);
    m_cvImage = rotated;

    if (m_isOverlayMode && !m_cvImage2.empty()) {
        updateOverlay();
    } else {
        updatePixmapFromMat();
        m_pixmapItem->setPixmap(m_originalPixmap);
        m_scene->setSceneRect(m_originalPixmap.rect());
    }

    emit scaleChanged();
}

void ImageViewer::rotateRight()
{
    if (!m_view->isEnabled() || m_cvImage.empty()) {
        return;
    }

    cv::Mat rotated;
    cv::transpose(m_cvImage, rotated);
    cv::flip(rotated, rotated, 1);
    m_cvImage = rotated;

    if (m_isOverlayMode && !m_cvImage2.empty()) {
        updateOverlay();
    } else {
        updatePixmapFromMat();
        m_pixmapItem->setPixmap(m_originalPixmap);
        m_scene->setSceneRect(m_originalPixmap.rect());
    }

    emit scaleChanged();
}

void ImageViewer::rotate180()
{
    if (!m_view->isEnabled() || m_cvImage.empty()) {
        return;
    }

    cv::Mat rotated;
    cv::flip(m_cvImage, rotated, -1);
    m_cvImage = rotated;

    if (m_isOverlayMode && !m_cvImage2.empty()) {
        updateOverlay();
    } else {
        updatePixmapFromMat();
        m_pixmapItem->setPixmap(m_originalPixmap);
        m_scene->setSceneRect(m_originalPixmap.rect());
    }

    emit scaleChanged();
}

void ImageViewer::flipHorizontal()
{
    if (!m_view->isEnabled() || m_cvImage.empty()) {
        return;
    }

    cv::Mat flipped;
    cv::flip(m_cvImage, flipped, 1);
    m_cvImage = flipped;

    if (m_isOverlayMode && !m_cvImage2.empty()) {
        updateOverlay();
    } else {
        updatePixmapFromMat();
        m_pixmapItem->setPixmap(m_originalPixmap);
        m_scene->setSceneRect(m_originalPixmap.rect());
    }

    emit scaleChanged();
}

void ImageViewer::flipVertical()
{
    if (!m_view->isEnabled() || m_cvImage.empty()) {
        return;
    }

    cv::Mat flipped;
    cv::flip(m_cvImage, flipped, 0);
    m_cvImage = flipped;

    if (m_isOverlayMode && !m_cvImage2.empty()) {
        updateOverlay();
    } else {
        updatePixmapFromMat();
        m_pixmapItem->setPixmap(m_originalPixmap);
        m_scene->setSceneRect(m_originalPixmap.rect());
    }

    emit scaleChanged();
}

bool ImageViewer::exportImage(const QString &fileName)
{
    if (!m_view->isEnabled() || fileName.isEmpty()) {
        return false;
    }

    cv::Mat imageToExport;
    if (m_isOverlayMode && !m_cvImage.empty() && !m_cvImage2.empty()) {
        imageToExport = computeOverlay();
        if (imageToExport.empty()) {
            return false;
        }
    } else if (!m_cvImage.empty()) {
        imageToExport = m_cvImage;
    } else {
        return false;
    }

    QFileInfo fileInfo(fileName);
    QString suffix = fileInfo.suffix().toLower();

    std::string ext;
    if (suffix == "png") {
        ext = ".png";
    } else if (suffix == "jpg" || suffix == "jpeg") {
        ext = ".jpg";
    } else if (suffix == "bmp") {
        ext = ".bmp";
    } else if (suffix == "tiff" || suffix == "tif") {
        ext = ".tiff";
    } else if (suffix == "webp") {
        ext = ".webp";
    } else {
        return false;
    }

    std::vector<uchar> buffer;
    cv::imencode(ext, imageToExport, buffer);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    file.close();

    return true;
}

QFuture<bool> ImageViewer::exportImageAsync(const QString &fileName)
{
    if (!m_view->isEnabled() || fileName.isEmpty()) {
        return QtConcurrent::run([]() { return false; });
    }

    cv::Mat imageToExport;
    if (m_isOverlayMode && !m_cvImage.empty() && !m_cvImage2.empty()) {
        imageToExport = computeOverlay();
        if (imageToExport.empty()) {
            return QtConcurrent::run([]() { return false; });
        }
    } else if (!m_cvImage.empty()) {
        imageToExport = m_cvImage.clone();
    } else {
        return QtConcurrent::run([]() { return false; });
    }

    QFileInfo fileInfo(fileName);
    QString suffix = fileInfo.suffix().toLower();

    std::string ext;
    if (suffix == "png") {
        ext = ".png";
    } else if (suffix == "jpg" || suffix == "jpeg") {
        ext = ".jpg";
    } else if (suffix == "bmp") {
        ext = ".bmp";
    } else if (suffix == "tiff" || suffix == "tif") {
        ext = ".tiff";
    } else if (suffix == "webp") {
        ext = ".webp";
    } else {
        return QtConcurrent::run([]() { return false; });
    }

    return QtConcurrent::run([imageToExport, fileName, ext]() {
        std::vector<uchar> buffer;
        cv::imencode(ext, imageToExport, buffer);

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }

        file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        file.close();

        return true;
    });
}

void ImageViewer::applyZoom(int percent)
{
    if (!m_view->isEnabled() || !m_pixmapItem) {
        return;
    }
    
    qreal scaleFactor = percent / 100.0;
    
    m_view->resetTransform();
    m_view->scale(scaleFactor, scaleFactor);
    
    updateScaleInfo();
    
    if (m_isFitToWindow) {
        m_isFitToWindow = false;
        emit fitToWindowChanged(false);
    }
}

void ImageViewer::updateCoordinates(QPointF scenePos)
{
    if (m_coordinateLabel) {
        int x = static_cast<int>(scenePos.x());
        int y = static_cast<int>(scenePos.y());
        
        m_coordinateLabel->setText(tr("坐标: (%1, %2)").arg(x).arg(y));
    }
}

bool ImageViewer::isEnabled() const
{
    return m_view->isEnabled();
}

QPixmap ImageViewer::originalPixmap() const
{
    return m_originalPixmap;
}

QGraphicsPixmapItem* ImageViewer::pixmapItem() const
{
    return m_pixmapItem;
}

void ImageViewer::setFitToWindow(bool fit)
{
    m_isFitToWindow = fit;
}

bool ImageViewer::isFitToWindow() const
{
    return m_isFitToWindow;
}

void ImageViewer::resizeEvent()
{
    if (m_isFitToWindow && m_pixmapItem) {
        m_view->fitInView(m_pixmapItem, Qt::KeepAspectRatio);
        updateScaleInfo();
    }
}

void ImageViewer::updateScaleInfo()
{
    if (!m_view->isEnabled() || !m_pixmapItem) {
        if (m_scaleLabel) {
            m_scaleLabel->setText("缩放:");
        }
        return;
    }
    
    QSize originalSize = m_originalPixmap.size();
    if (originalSize.width() <= 0 || originalSize.height() <= 0) {
        if (m_scaleLabel) {
            m_scaleLabel->setText("缩放:");
        }
        return;
    }
    
    qreal currentScale = m_view->transform().m11() * 100;
    
    if (m_scaleLabel) {
        m_scaleLabel->setText(tr("缩放:"));
    }
    
    if (m_zoomSlider) {
        m_zoomSlider->setValue(qRound(qBound(qreal(1), currentScale, qreal(3200))));
    }
    if (m_zoomSpinBox) {
        m_zoomSpinBox->setValue(qRound(qBound(qreal(1), currentScale, qreal(3200))));
    }
}

void ImageViewer::updateSizeInfo()
{
    if (m_pixmapItem && m_view->isEnabled()) {
        int width = m_originalPixmap.width();
        int height = m_originalPixmap.height();
        if (m_sizeLabel) {
            m_sizeLabel->setText(tr("尺寸: %1x%2").arg(width).arg(height));
        }
        
        if (m_imageSizeLabel) {
            QString sizeStr;
            if (m_fileSize < 1024) {
                sizeStr = tr("%1 B").arg(m_fileSize);
            } else if (m_fileSize < 1024 * 1024) {
                sizeStr = tr("%1 KB").arg(m_fileSize / 1024.0, 0, 'f', 2);
            } else if (m_fileSize < 1024 * 1024 * 1024) {
                sizeStr = tr("%1 MB").arg(m_fileSize / (1024.0 * 1024.0), 0, 'f', 2);
            } else {
                sizeStr = tr("%1 GB").arg(m_fileSize / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
            }
            m_imageSizeLabel->setText(tr("图片大小: %1").arg(sizeStr));
        }
    } else {
        if (m_sizeLabel) {
            m_sizeLabel->setText("尺寸: 0x0");
        }
        if (m_imageSizeLabel) {
            m_imageSizeLabel->setText("图片大小: 0 B");
        }
    }
}

void ImageViewer::updatePixmapFromMat()
{
    if (m_cvImage.empty()) {
        return;
    }
    
    cv::Mat cvImageRGB;
    if (m_cvImage.channels() == 3) {
        cv::cvtColor(m_cvImage, cvImageRGB, cv::COLOR_BGR2RGB);
    } else if (m_cvImage.channels() == 4) {
        cv::cvtColor(m_cvImage, cvImageRGB, cv::COLOR_BGRA2RGBA);
    } else {
        cvImageRGB = m_cvImage.clone();
    }
    
    QImage::Format format;
    if (cvImageRGB.channels() == 1) {
        format = QImage::Format_Grayscale8;
    } else if (cvImageRGB.channels() == 4) {
        format = QImage::Format_RGBA8888;
    } else {
        format = QImage::Format_RGB888;
    }
    
    QImage qImage(
        cvImageRGB.data,
        cvImageRGB.cols,
        cvImageRGB.rows,
        static_cast<int>(cvImageRGB.step),
        format
    );
    
    m_originalPixmap = QPixmap::fromImage(qImage.copy());
}

void ImageViewer::loadImagesFromDirectory(const QString &directoryPath)
{
    m_imageList.clear();
    
    QDir dir(directoryPath);
    if (!dir.exists()) {
        return;
    }
    
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.tiff" << "*.tif" << "*.webp";
    
    dir.setNameFilters(filters);
    dir.setSorting(QDir::Name | QDir::IgnoreCase);
    
    QFileInfoList fileList = dir.entryInfoList(QDir::Files);
    for (const QFileInfo &fileInfo : fileList) {
        m_imageList.append(fileInfo.absoluteFilePath());
    }
}

void ImageViewer::saveCurrentPosition()
{
    if (!m_currentDirectory.isEmpty() && m_currentImageIndex >= 0) {
        m_settings->setValue("lastDirectory", m_currentDirectory);
        m_settings->setValue("lastImageIndex", m_currentImageIndex);
    }
}

void ImageViewer::loadSavedPosition()
{
    QString savedDirectory = m_settings->value("lastDirectory").toString();
    if (savedDirectory == m_currentDirectory) {
        int savedIndex = m_settings->value("lastImageIndex", -1).toInt();
        if (savedIndex >= 0 && savedIndex < m_imageList.size()) {
            m_currentImageIndex = savedIndex;
        }
    }
}

void ImageViewer::updateImageIndexLabel()
{
    if (m_imageIndexLabel && !m_imageList.isEmpty()) {
        m_imageIndexLabel->setText(tr("%1/%2").arg(m_currentImageIndex + 1).arg(m_imageList.size()));
    } else if (m_imageIndexLabel) {
        m_imageIndexLabel->setText("0/0");
    }
}

void ImageViewer::nextImage()
{
    if (m_imageList.isEmpty()) {
        return;
    }
    
    saveCurrentPosition();
    
    m_currentImageIndex++;
    if (m_currentImageIndex >= m_imageList.size()) {
        m_currentImageIndex = 0;
    }
    
    openImage(m_imageList[m_currentImageIndex]);
}

void ImageViewer::previousImage()
{
    if (m_imageList.isEmpty()) {
        return;
    }

    saveCurrentPosition();

    m_currentImageIndex--;
    if (m_currentImageIndex < 0) {
        m_currentImageIndex = m_imageList.size() - 1;
    }

    openImage(m_imageList[m_currentImageIndex]);
}

// Overlay mode implementation

void ImageViewer::enableOverlayMode(bool enable)
{
    m_isOverlayMode = enable;
    emit overlayModeChanged(enable);

    if (enable && !m_cvImage2.empty()) {
        updateOverlay();
    } else if (!enable && !m_cvImage.empty()) {
        // Restore original image 1
        updatePixmapFromMat();
        if (m_pixmapItem) {
            m_pixmapItem->setPixmap(m_originalPixmap);
            m_scene->setSceneRect(m_originalPixmap.rect());
        }
    }
}

bool ImageViewer::isOverlayMode() const
{
    return m_isOverlayMode;
}

bool ImageViewer::loadSecondImage(const QString &fileName)
{
    if (fileName.isEmpty()) {
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(nullptr, tr("错误"), tr("无法打开图片文件: %1").arg(fileName));
        return false;
    }

    QByteArray fileData = file.readAll();
    file.close();

    cv::Mat matData(1, fileData.size(), CV_8U, (void*)fileData.data());

    cv::Mat cvImage = cv::imdecode(matData, cv::IMREAD_UNCHANGED);
    if (cvImage.empty()) {
        QMessageBox::warning(nullptr, tr("错误"), tr("无法解码图片文件: %1").arg(fileName));
        return false;
    }

    m_cvImage2 = cvImage;
    m_currentImage2Path = fileName;

    emit secondImageLoaded(fileName);

    if (m_isOverlayMode) {
        updateOverlay();
    }

    return true;
}

void ImageViewer::clearSecondImage()
{
    m_cvImage2.release();
    m_currentImage2Path.clear();

    emit secondImageCleared();

    if (m_isOverlayMode && !m_cvImage.empty()) {
        // Restore original image 1
        updatePixmapFromMat();
        if (m_pixmapItem) {
            m_pixmapItem->setPixmap(m_originalPixmap);
            m_scene->setSceneRect(m_originalPixmap.rect());
        }
    }
}

void ImageViewer::setAlpha1(double alpha)
{
    m_alpha1 = qBound(0.0, alpha, 1.0);

    if (m_isOverlayMode && !m_cvImage.empty() && !m_cvImage2.empty()) {
        m_overlayUpdateTimer->start(100);
    }
}

void ImageViewer::setAlpha2(double alpha)
{
    m_alpha2 = qBound(0.0, alpha, 1.0);

    if (m_isOverlayMode && !m_cvImage.empty() && !m_cvImage2.empty()) {
        m_overlayUpdateTimer->start(100);
    }
}

double ImageViewer::getAlpha1() const
{
    return m_alpha1;
}

double ImageViewer::getAlpha2() const
{
    return m_alpha2;
}

void ImageViewer::alignImages(const cv::Mat &img1, const cv::Mat &img2,
                               cv::Mat &aligned1, cv::Mat &aligned2)
{
    // Unify number of channels
    cv::Mat img1_unified, img2_unified;

    // Convert to RGB if needed
    if (img1.channels() == 1) {
        cv::cvtColor(img1, img1_unified, cv::COLOR_GRAY2BGR);
    } else if (img1.channels() == 4) {
        cv::cvtColor(img1, img1_unified, cv::COLOR_BGRA2BGR);
    } else {
        img1_unified = img1.clone();
    }

    if (img2.channels() == 1) {
        cv::cvtColor(img2, img2_unified, cv::COLOR_GRAY2BGR);
    } else if (img2.channels() == 4) {
        cv::cvtColor(img2, img2_unified, cv::COLOR_BGRA2BGR);
    } else {
        img2_unified = img2.clone();
    }

    // Get maximum dimensions
    int maxWidth = std::max(img1_unified.cols, img2_unified.cols);
    int maxHeight = std::max(img1_unified.rows, img2_unified.rows);

    // Create canvases with maximum size
    aligned1 = cv::Mat::zeros(maxHeight, maxWidth, CV_8UC3);
    aligned2 = cv::Mat::zeros(maxHeight, maxWidth, CV_8UC3);

    // Center align images
    int x1 = (maxWidth - img1_unified.cols) / 2;
    int y1 = (maxHeight - img1_unified.rows) / 2;
    img1_unified.copyTo(aligned1(cv::Rect(x1, y1, img1_unified.cols, img1_unified.rows)));

    int x2 = (maxWidth - img2_unified.cols) / 2;
    int y2 = (maxHeight - img2_unified.rows) / 2;
    img2_unified.copyTo(aligned2(cv::Rect(x2, y2, img2_unified.cols, img2_unified.rows)));
}

cv::Mat ImageViewer::computeOverlay()
{
    if (m_cvImage.empty() || m_cvImage2.empty()) {
        return cv::Mat();
    }

    cv::Mat aligned1, aligned2;
    alignImages(m_cvImage, m_cvImage2, aligned1, aligned2);

    cv::Mat result;
    cv::addWeighted(aligned1, m_alpha1, aligned2, m_alpha2, 0, result);

    return result;
}

void ImageViewer::updateOverlay()
{
    if (!m_isOverlayMode || m_cvImage.empty() || m_cvImage2.empty()) {
        return;
    }

    m_overlayResult = computeOverlay();

    if (m_overlayResult.empty()) {
        return;
    }

    // Convert overlay result to QPixmap
    cv::Mat cvImageRGB;
    if (m_overlayResult.channels() == 3) {
        cv::cvtColor(m_overlayResult, cvImageRGB, cv::COLOR_BGR2RGB);
    } else if (m_overlayResult.channels() == 4) {
        cv::cvtColor(m_overlayResult, cvImageRGB, cv::COLOR_BGRA2RGBA);
    } else {
        cvImageRGB = m_overlayResult.clone();
    }

    QImage::Format format;
    if (cvImageRGB.channels() == 1) {
        format = QImage::Format_Grayscale8;
    } else if (cvImageRGB.channels() == 4) {
        format = QImage::Format_RGBA8888;
    } else {
        format = QImage::Format_RGB888;
    }

    QImage qImage(
        cvImageRGB.data,
        cvImageRGB.cols,
        cvImageRGB.rows,
        static_cast<int>(cvImageRGB.step),
        format
    );

    QPixmap overlayPixmap = QPixmap::fromImage(qImage.copy());

    if (m_pixmapItem) {
        m_pixmapItem->setPixmap(overlayPixmap);
        m_scene->setSceneRect(overlayPixmap.rect());
    }
}
