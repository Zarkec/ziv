#include "imageviewer.h"
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
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
    , m_zoomSlider(nullptr)
    , m_zoomSpinBox(nullptr)
    , m_isFitToWindow(false)
{
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

void ImageViewer::setZoomSlider(QSlider *slider)
{
    m_zoomSlider = slider;
}

void ImageViewer::setZoomSpinBox(QSpinBox *spinBox)
{
    m_zoomSpinBox = spinBox;
}

void ImageViewer::openImage(const QString &fileName)
{
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(nullptr, tr("错误"), tr("无法打开图片文件: %1").arg(fileName));
        return;
    }
    
    QByteArray fileData = file.readAll();
    file.close();
    
    cv::Mat matData(1, fileData.size(), CV_8U, (void*)fileData.data());
    
    cv::Mat cvImage = cv::imdecode(matData, cv::IMREAD_UNCHANGED);
    if (cvImage.empty()) {
        QMessageBox::warning(nullptr, tr("错误"), tr("无法解码图片文件: %1").arg(fileName));
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
    
    emit imageLoaded(fileName);
}

void ImageViewer::zoomIn()
{
    if (!m_view->isEnabled()) {
        return;
    }
    
    qreal currentScale = m_view->transform().m11() * 100;
    
    if (currentScale < 100000) {
        qreal maxScaleFactor = 1.2;
        if (currentScale * 1.2 > 100000) {
            maxScaleFactor = 100000 / currentScale;
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
    
    updatePixmapFromMat();
    m_pixmapItem->setPixmap(m_originalPixmap);
    m_scene->setSceneRect(m_originalPixmap.rect());
    
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
    
    updatePixmapFromMat();
    m_pixmapItem->setPixmap(m_originalPixmap);
    m_scene->setSceneRect(m_originalPixmap.rect());
    
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
    
    updatePixmapFromMat();
    m_pixmapItem->setPixmap(m_originalPixmap);
    m_scene->setSceneRect(m_originalPixmap.rect());
    
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
    
    updatePixmapFromMat();
    m_pixmapItem->setPixmap(m_originalPixmap);
    m_scene->setSceneRect(m_originalPixmap.rect());
    
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
    
    updatePixmapFromMat();
    m_pixmapItem->setPixmap(m_originalPixmap);
    m_scene->setSceneRect(m_originalPixmap.rect());
    
    emit scaleChanged();
}

bool ImageViewer::exportImage(const QString &fileName)
{
    if (!m_view->isEnabled() || m_cvImage.empty() || fileName.isEmpty()) {
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
    cv::imencode(ext, m_cvImage, buffer);
    
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
    if (!m_view->isEnabled() || m_cvImage.empty() || fileName.isEmpty()) {
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
    
    cv::Mat image = m_cvImage.clone();
    
    return QtConcurrent::run([image, fileName, ext]() {
        std::vector<uchar> buffer;
        cv::imencode(ext, image, buffer);
        
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
    
    QRectF viewRect = m_view->mapFromScene(m_pixmapItem->sceneBoundingRect()).boundingRect();
    QSize originalSize = m_originalPixmap.size();
    
    qreal scaleX = viewRect.width() / originalSize.width();
    qreal scaleY = viewRect.height() / originalSize.height();
    qreal scale = qMin(scaleX, scaleY) * 100;
    
    if (m_scaleLabel) {
        m_scaleLabel->setText(tr("缩放:"));
    }
    
    if (m_zoomSlider) {
        m_zoomSlider->setValue(qRound(scale));
    }
    if (m_zoomSpinBox) {
        m_zoomSpinBox->setValue(qRound(scale));
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
    } else {
        if (m_sizeLabel) {
            m_sizeLabel->setText("尺寸: 0x0");
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
