#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QMessageBox>
#include <QCursor>
#include <QSlider>
#include <QSpinBox>
#include <QHBoxLayout>

// ImageGraphicsView 类实现
ImageGraphicsView::ImageGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{}

void ImageGraphicsView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    
    QPointF viewPos = event->pos();
    QPointF scenePos = mapToScene(viewPos.toPoint());
    emit mousePressed(scenePos);
}

void ImageGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    
    QPointF viewPos = event->pos();
    QPointF scenePos = mapToScene(viewPos.toPoint());
    emit mouseMoved(scenePos);
}

void ImageGraphicsView::leaveEvent(QEvent *event)
{
    QGraphicsView::leaveEvent(event);
    emit mouseLeft();
}

void ImageGraphicsView::enterEvent(QEnterEvent *event)
{
    QGraphicsView::enterEvent(event);
    
    QPointF viewPos = event->position();
    QPointF scenePos = mapToScene(viewPos.toPoint());
    emit mouseEntered(scenePos);
}

void ImageGraphicsView::wheelEvent(QWheelEvent *event)
{
    // 检查是否按下了Ctrl键
    if (event->modifiers() & Qt::ControlModifier) {
        // 计算缩放因子
        qreal scaleFactor = 1.15;
        if (event->angleDelta().y() < 0) {
            scaleFactor = 1.0 / scaleFactor;
        }
        
        // 检查缩放限制
        qreal currentScale = transform().m11() * 100;
        qreal newScale = currentScale * scaleFactor;
        
        // 确保缩放后不超过限制
        if ((scaleFactor > 1 && currentScale < 100000) || (scaleFactor < 1 && currentScale > 1)) {
            // 计算实际允许的缩放因子
            qreal actualScaleFactor = scaleFactor;
            if (scaleFactor > 1 && currentScale < 100000) {
                actualScaleFactor = 100000 / currentScale;
                // 如果最大缩放因子大于1.15，则使用1.15，否则使用计算出的最大缩放因子
                actualScaleFactor = qMin(actualScaleFactor, 1.15);
            } else if (scaleFactor < 1 && currentScale > 1) {
                actualScaleFactor = 1.0 / 1.15;
                // 确保缩放后不低于1%
                if (currentScale * actualScaleFactor < 1) {
                    actualScaleFactor = 1.0 / currentScale;
                }
            }
            
            // 执行缩放操作
            scale(actualScaleFactor, actualScaleFactor);
            
            // 发射缩放变化信号
            emit scaleChanged();
            
            // 阻止默认的滚动行为
            event->accept();
        }
    } else {
        // 调用默认的滚轮事件处理（滚动）
        QGraphicsView::wheelEvent(event);
    }
}

void ImageGraphicsView::keyPressEvent(QKeyEvent *event)
{
    QGraphicsView::keyPressEvent(event);
    
    if (event->key() == Qt::Key_Shift) {
        emit shiftPressed();
    }
}

void ImageGraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    QGraphicsView::keyReleaseEvent(event);
    
    if (event->key() == Qt::Key_Shift) {
        emit shiftReleased();
    }
}

// MainWindow 类实现
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_graphicsView(nullptr)
    , m_graphicsScene(nullptr)
    , m_pixmapItem(nullptr)
    , m_coordinateLabel(nullptr)
    , m_scaleLabel(nullptr)
    , m_sizeLabel(nullptr)
    , m_fitToWindowAction(nullptr)
    , m_measureAction(nullptr)
    , m_isFitToWindow(false)
    , m_isMeasureMode(false)
    , m_isMeasureCompleted(false)
    , m_isShiftPressed(false)
    , m_measureLine(nullptr)
    , m_measureText(nullptr)
    , m_zoomSlider(nullptr)
    , m_zoomSpinBox(nullptr)
{
    setupUI();
    setupActions();
    setupConnections();
    
    // 初始状态禁用缩放和平移功能
    m_graphicsView->setEnabled(false);
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    // 创建中心部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // 创建图形视图和场景
    m_graphicsScene = new QGraphicsScene(this);
    m_graphicsView = new ImageGraphicsView(this);
    m_graphicsView->setScene(m_graphicsScene);
    
    // 设置图形视图属性
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    m_graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);
    m_graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    m_graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    m_graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    
    // 添加图形视图到布局
    mainLayout->addWidget(m_graphicsView);
    
    // 创建状态栏
    m_coordinateLabel = new QLabel("坐标: (0, 0)", this);
    m_scaleLabel = new QLabel("缩放:", this);
    m_sizeLabel = new QLabel("尺寸: 0x0", this);
    
    // 将坐标标签添加到左侧
    statusBar()->addWidget(m_coordinateLabel);
    
    // 将尺寸标签添加到右侧
    statusBar()->addPermanentWidget(m_sizeLabel);
    
    // 将缩放标签和控制组件合并添加到右侧
    statusBar()->addPermanentWidget(m_scaleLabel);
    
    // 创建滑动条
    m_zoomSlider = new QSlider(Qt::Horizontal, this);
    m_zoomSlider->setMinimum(1);
    m_zoomSlider->setMaximum(10000);
    m_zoomSlider->setValue(100);
    m_zoomSlider->setEnabled(false);
    m_zoomSlider->setFixedWidth(150);
    statusBar()->addPermanentWidget(m_zoomSlider);
    
    // 创建文本输入框
    m_zoomSpinBox = new QSpinBox(this);
    m_zoomSpinBox->setMinimum(1);
    m_zoomSpinBox->setMaximum(10000);
    m_zoomSpinBox->setValue(100);
    m_zoomSpinBox->setSuffix("%");
    m_zoomSpinBox->setFixedWidth(80);
    m_zoomSpinBox->setEnabled(false);
    statusBar()->addPermanentWidget(m_zoomSpinBox);
    
    // 设置窗口大小
    resize(800, 600);
    setWindowTitle("图片查看器");
}

void MainWindow::setupActions()
{
    // 创建菜单
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    QMenu *viewMenu = menuBar()->addMenu("视图(&V)");
    
    // 文件菜单动作
    QAction *openAction = new QAction("打开(&O)", this);
    openAction->setShortcut(QKeySequence::Open);
    fileMenu->addAction(openAction);
    
    // 视图菜单动作
    QAction *zoomInAction = new QAction("放大(&+)", this);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    
    QAction *zoomOutAction = new QAction("缩小(&-)", this);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    
    m_fitToWindowAction = new QAction("适应窗口(&W)", this);
    m_fitToWindowAction->setCheckable(true);
    
    QAction *originalSizeAction = new QAction("原始大小(&1)", this);
    originalSizeAction->setShortcut(tr("Ctrl+1"));
    
    // 添加旋转动作
    QAction *rotateLeftAction = new QAction("向左旋转(&L)", this);
    rotateLeftAction->setShortcut(tr("Ctrl+L"));
    
    QAction *rotateRightAction = new QAction("向右旋转(&R)", this);
    rotateRightAction->setShortcut(tr("Ctrl+R"));
    
    QAction *rotate180Action = new QAction("旋转180度(&O)", this);
    rotate180Action->setShortcut(tr("Ctrl+O"));
    
    // 添加测量模式动作
    m_measureAction = new QAction("测量模式(&M)", this);
    m_measureAction->setCheckable(true);
    m_measureAction->setShortcut(tr("Ctrl+M"));
    
    viewMenu->addAction(zoomInAction);
    viewMenu->addAction(zoomOutAction);
    viewMenu->addSeparator();
    viewMenu->addAction(rotateLeftAction);
    viewMenu->addAction(rotateRightAction);
    viewMenu->addAction(rotate180Action);
    viewMenu->addSeparator();
    viewMenu->addAction(m_measureAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_fitToWindowAction);
    viewMenu->addAction(originalSizeAction);
    
    // 创建工具栏
    QToolBar *toolBar = addToolBar("查看工具栏");
    toolBar->addAction(openAction);
    toolBar->addSeparator();
    toolBar->addAction(zoomInAction);
    toolBar->addAction(zoomOutAction);
    toolBar->addSeparator();
    toolBar->addAction(rotateLeftAction);
    toolBar->addAction(rotateRightAction);
    toolBar->addAction(rotate180Action);
    toolBar->addSeparator();
    toolBar->addAction(m_measureAction);
    toolBar->addSeparator();
    toolBar->addAction(m_fitToWindowAction);
    toolBar->addAction(originalSizeAction);
    
    // 连接信号槽
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(rotateLeftAction, &QAction::triggered, this, &MainWindow::rotateLeft);
    connect(rotateRightAction, &QAction::triggered, this, &MainWindow::rotateRight);
    connect(rotate180Action, &QAction::triggered, this, &MainWindow::rotate180);
    connect(m_measureAction, &QAction::triggered, this, &MainWindow::toggleMeasureMode);
    connect(m_fitToWindowAction, &QAction::triggered, this, &MainWindow::fitToWindow);
    connect(originalSizeAction, &QAction::triggered, this, &MainWindow::originalSize);
}

void MainWindow::setupConnections()
{
    // 缩放控件之间的同步
    connect(m_zoomSlider, &QSlider::valueChanged, this, [this](int value) {
        m_zoomSpinBox->setValue(value);
        applyZoom(value);
    });
    
    connect(m_zoomSpinBox, &QSpinBox::valueChanged, this, [this](int value) {
        m_zoomSlider->setValue(value);
        applyZoom(value);
    });
    
    // 连接自定义视图的信号
    connect(m_graphicsView, &ImageGraphicsView::mouseMoved, this, [this](QPointF scenePos) {
        // 确保坐标在图片范围内
        if (m_pixmapItem && m_graphicsScene->sceneRect().contains(scenePos)) {
            updateCoordinates(scenePos);
            handleMouseMove(scenePos);
        }
    });
    
    connect(m_graphicsView, &ImageGraphicsView::mousePressed, this, [this](QPointF scenePos) {
        // 确保坐标在图片范围内
        if (m_pixmapItem && m_graphicsScene->sceneRect().contains(scenePos)) {
            handleMousePress(scenePos);
        }
    });
    
    // 连接Shift键状态信号
    connect(m_graphicsView, &ImageGraphicsView::shiftPressed, this, [this]() {
        m_isShiftPressed = true;
        // 如果正在测量，更新当前测量线
        if (m_isMeasureMode && m_measureLine != nullptr && !m_isMeasureCompleted) {
            // 重新处理当前鼠标位置，应用水平/竖直约束
            QPoint cursorPos = m_graphicsView->mapFromGlobal(QCursor::pos());
            QPointF scenePos = m_graphicsView->mapToScene(cursorPos);
            handleMouseMove(scenePos);
        }
    });
    
    connect(m_graphicsView, &ImageGraphicsView::shiftReleased, this, [this]() {
        m_isShiftPressed = false;
        // 如果正在测量，更新当前测量线
        if (m_isMeasureMode && m_measureLine != nullptr && !m_isMeasureCompleted) {
            // 重新处理当前鼠标位置，取消水平/竖直约束
            QPoint cursorPos = m_graphicsView->mapFromGlobal(QCursor::pos());
            QPointF scenePos = m_graphicsView->mapToScene(cursorPos);
            handleMouseMove(scenePos);
        }
    });
    
    connect(m_graphicsView, &ImageGraphicsView::mouseLeft, this, [this]() {
        m_coordinateLabel->setText("坐标: (0, 0)");
    });
    
    connect(m_graphicsView, &ImageGraphicsView::mouseEntered, this, [this](QPointF scenePos) {
        if (m_pixmapItem && m_graphicsScene->sceneRect().contains(scenePos)) {
            updateCoordinates(scenePos);
        }
    });
    
    // 连接自定义的scaleChanged信号，用于更新缩放信息
    connect(m_graphicsView, &ImageGraphicsView::scaleChanged, this, [this]() {
        updateScaleInfo();
        
        // 更新测量线的视觉参数，保持适当比例
        updateMeasurementScale();
        
        // 如果处于适应窗口模式，退出该模式
        if (m_isFitToWindow) {
            m_isFitToWindow = false;
            m_fitToWindowAction->setChecked(false);
        }
    });
}

void MainWindow::openImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, 
        tr("打开图片"), 
        QString(), 
        tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif);;所有文件 (*.*)")
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QPixmap pixmap;
    QImage image;
    
    // 使用Qt读取文件数据，避免OpenCV中文路径问题
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("错误"), tr("无法打开图片文件: %1").arg(fileName));
        return;
    }
    
    QByteArray fileData = file.readAll();
    file.close();
    
    // 将QByteArray转换为cv::Mat
    cv::Mat matData(1, fileData.size(), CV_8U, (void*)fileData.data());
    
    // 使用imdecode从内存解码图片
    cv::Mat cvImage = cv::imdecode(matData, cv::IMREAD_UNCHANGED);
    if (cvImage.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("无法解码图片文件: %1").arg(fileName));
        return;
    }
    
    // 转换OpenCV的BGR格式到RGB格式
    cv::Mat cvImageRGB;
    if (cvImage.channels() == 3) {
        cv::cvtColor(cvImage, cvImageRGB, cv::COLOR_BGR2RGB);
    } else if (cvImage.channels() == 4) {
        cv::cvtColor(cvImage, cvImageRGB, cv::COLOR_BGRA2RGBA);
    } else {
        // 单通道图片直接使用
        cvImageRGB = cvImage.clone();
    }
    
    // 将cv::Mat转换为QImage
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
    
    // 深拷贝QImage，确保数据有效
    image = qImage.copy();
    
    // 转换为QPixmap
    pixmap = QPixmap::fromImage(image);
    m_originalPixmap = pixmap;
    
    // 获取图片信息
    QSize imageSize = image.size();
    qint64 fileSize = fileData.size();
    
    // 如果已有图片项，先移除
    if (m_pixmapItem) {
        m_graphicsScene->removeItem(m_pixmapItem);
        delete m_pixmapItem;
        m_pixmapItem = nullptr;
    }
    
    // 清除测量线
    clearMeasurement();
    
    // 创建新的图片项
    m_pixmapItem = m_graphicsScene->addPixmap(pixmap);
    m_graphicsScene->setSceneRect(pixmap.rect());
    
    // 启用图形视图和缩放控件
    m_graphicsView->setEnabled(true);
    m_zoomSlider->setEnabled(true);
    m_zoomSpinBox->setEnabled(true);
    
    // 默认适应窗口
    m_isFitToWindow = true;
    m_fitToWindowAction->setChecked(true);
    m_graphicsView->fitInView(m_pixmapItem, Qt::KeepAspectRatio);
    
    // 更新图片尺寸信息和缩放信息
    updateSizeInfo();
    updateScaleInfo();
    
    // 更新窗口标题
    setWindowTitle(tr("图片查看器 - %1").arg(fileName));
}

void MainWindow::zoomIn()
{
    if (!m_graphicsView->isEnabled()) {
        return;
    }
    
    // 获取当前缩放比例
    qreal currentScale = m_graphicsView->transform().m11() * 100;
    
    // 检查是否超过最大缩放限制（100000%）
    if (currentScale < 100000) {
        // 计算实际允许的最大缩放因子
        qreal maxScaleFactor = 1.2;
        if (currentScale * 1.2 > 100000) {
            maxScaleFactor = 100000 / currentScale;
        }
        m_graphicsView->scale(maxScaleFactor, maxScaleFactor);
        updateScaleInfo();
        
        // 如果处于适应窗口模式，退出该模式
        if (m_isFitToWindow) {
            m_isFitToWindow = false;
            m_fitToWindowAction->setChecked(false);
        }
    }
}

void MainWindow::zoomOut()
{
    if (!m_graphicsView->isEnabled()) {
        return;
    }
    
    // 获取当前缩放比例
    qreal currentScale = m_graphicsView->transform().m11() * 100;
    
    // 检查是否低于最小缩放限制（1%）
    if (currentScale > 1) {
        m_graphicsView->scale(1.0 / 1.2, 1.0 / 1.2);
        updateScaleInfo();
        
        // 如果处于适应窗口模式，退出该模式
        if (m_isFitToWindow) {
            m_isFitToWindow = false;
            m_fitToWindowAction->setChecked(false);
        }
    }
}

void MainWindow::fitToWindow()
{
    if (!m_graphicsView->isEnabled() || !m_pixmapItem) {
        return;
    }
    
    m_isFitToWindow = m_fitToWindowAction->isChecked();
    
    if (m_isFitToWindow) {
        m_graphicsView->fitInView(m_pixmapItem, Qt::KeepAspectRatio);
    } else {
        originalSize();
    }
    
    updateScaleInfo();
}

void MainWindow::originalSize()
{
    if (!m_graphicsView->isEnabled()) {
        return;
    }
    
    m_graphicsView->resetTransform();
    updateScaleInfo();
    
    // 确保适应窗口模式未选中
    m_isFitToWindow = false;
    m_fitToWindowAction->setChecked(false);
}

void MainWindow::updateCoordinates(QPointF scenePos)
{
    // 将场景坐标转换为图片坐标（像素为单位）
    int x = static_cast<int>(scenePos.x());
    int y = static_cast<int>(scenePos.y());
    
    m_coordinateLabel->setText(tr("坐标: (%1, %2)").arg(x).arg(y));
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    
    // 如果处于适应窗口模式，重新调整大小
    if (m_isFitToWindow && m_pixmapItem) {
        m_graphicsView->fitInView(m_pixmapItem, Qt::KeepAspectRatio);
        updateScaleInfo();
    }
}

void MainWindow::applyZoom(int percent)
{
    if (!m_graphicsView->isEnabled() || !m_pixmapItem) {
        return;
    }
    
    // 计算缩放因子
    qreal scaleFactor = percent / 100.0;
    
    // 重置变换并应用新的缩放
    m_graphicsView->resetTransform();
    m_graphicsView->scale(scaleFactor, scaleFactor);
    
    // 更新缩放信息
    updateScaleInfo();
    
    // 确保退出适应窗口模式
    if (m_isFitToWindow) {
        m_isFitToWindow = false;
        m_fitToWindowAction->setChecked(false);
    }
}

void MainWindow::updateScaleInfo()
{
    if (!m_graphicsView->isEnabled() || !m_pixmapItem) {
        m_scaleLabel->setText("缩放:");
        return;
    }
    
    // 获取图片在视图中的显示矩形
    QRectF viewRect = m_graphicsView->mapFromScene(m_pixmapItem->sceneBoundingRect()).boundingRect();
    
    // 获取原始图片尺寸
    QSize originalSize = m_originalPixmap.size();
    
    // 计算缩放比例（取宽度和高度中较小的那个，因为fitInView使用KeepAspectRatio）
    qreal scaleX = viewRect.width() / originalSize.width();
    qreal scaleY = viewRect.height() / originalSize.height();
    qreal scale = qMin(scaleX, scaleY) * 100;
    
    m_scaleLabel->setText(tr("缩放:"));
    
    // 更新缩放控件的值
    m_zoomSlider->setValue(qRound(scale));
    m_zoomSpinBox->setValue(qRound(scale));
}

void MainWindow::updateSizeInfo()
{
    if (m_pixmapItem && m_graphicsView->isEnabled()) {
        // 获取当前显示图片尺寸
        int width = m_originalPixmap.width();
        int height = m_originalPixmap.height();
        m_sizeLabel->setText(tr("尺寸: %1x%2").arg(width).arg(height));
    } else {
        m_sizeLabel->setText("尺寸: 0x0");
    }
}

void MainWindow::rotateLeft()
{
    if (!m_graphicsView->isEnabled() || !m_pixmapItem) {
        return;
    }
    
    // 将QPixmap转换为QImage进行旋转
    QImage image = m_originalPixmap.toImage();
    
    // 向左旋转90度
    QTransform transform;
    transform.rotate(-90);
    QImage rotatedImage = image.transformed(transform, Qt::SmoothTransformation);
    
    // 更新原始图片和场景
    m_originalPixmap = QPixmap::fromImage(rotatedImage);
    m_pixmapItem->setPixmap(m_originalPixmap);
    m_graphicsScene->setSceneRect(m_originalPixmap.rect());
    
    // 清除测量线
    clearMeasurement();
    
    // 如果处于适应窗口模式，重新调整大小
    if (m_isFitToWindow) {
        m_graphicsView->fitInView(m_pixmapItem, Qt::KeepAspectRatio);
    }
    
    // 更新尺寸信息和缩放信息
    updateSizeInfo();
    updateScaleInfo();
}

void MainWindow::rotateRight()
{
    if (!m_graphicsView->isEnabled() || !m_pixmapItem) {
        return;
    }
    
    // 将QPixmap转换为QImage进行旋转
    QImage image = m_originalPixmap.toImage();
    
    // 向右旋转90度
    QTransform transform;
    transform.rotate(90);
    QImage rotatedImage = image.transformed(transform, Qt::SmoothTransformation);
    
    // 更新原始图片和场景
    m_originalPixmap = QPixmap::fromImage(rotatedImage);
    m_pixmapItem->setPixmap(m_originalPixmap);
    m_graphicsScene->setSceneRect(m_originalPixmap.rect());
    
    // 清除测量线
    clearMeasurement();
    
    // 如果处于适应窗口模式，重新调整大小
    if (m_isFitToWindow) {
        m_graphicsView->fitInView(m_pixmapItem, Qt::KeepAspectRatio);
    }
    
    // 更新尺寸信息和缩放信息
    updateSizeInfo();
    updateScaleInfo();
}

void MainWindow::rotate180()
{
    if (!m_graphicsView->isEnabled() || !m_pixmapItem) {
        return;
    }
    
    // 将QPixmap转换为QImage进行旋转
    QImage image = m_originalPixmap.toImage();
    
    // 旋转180度
    QTransform transform;
    transform.rotate(180);
    QImage rotatedImage = image.transformed(transform, Qt::SmoothTransformation);
    
    // 更新原始图片和场景
    m_originalPixmap = QPixmap::fromImage(rotatedImage);
    m_pixmapItem->setPixmap(m_originalPixmap);
    m_graphicsScene->setSceneRect(m_originalPixmap.rect());
    
    // 清除测量线
    clearMeasurement();
    
    // 如果处于适应窗口模式，重新调整大小
    if (m_isFitToWindow) {
        m_graphicsView->fitInView(m_pixmapItem, Qt::KeepAspectRatio);
    }
    
    // 更新尺寸信息和缩放信息
    updateSizeInfo();
    updateScaleInfo();
}

void MainWindow::toggleMeasureMode()
{
    m_isMeasureMode = m_measureAction->isChecked();
    
    if (!m_isMeasureMode) {
        clearMeasurement();
        // 退出测量模式，恢复手型光标和拖拽模式
        m_graphicsView->setCursor(Qt::ArrowCursor);
        m_graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    } else {
        // 进入测量模式，设置十字光标并禁用拖拽
        m_graphicsView->setCursor(Qt::CrossCursor);
        m_graphicsView->setDragMode(QGraphicsView::NoDrag);
    }
}

void MainWindow::handleMousePress(QPointF scenePos)
{
    if (!m_isMeasureMode) {
        return;
    }
    
    if (m_measureLine == nullptr) {
        // 第一次点击：设置起点，创建测量线
        m_measureStart = scenePos;
        m_measureEnd = scenePos;
        m_isMeasureCompleted = false;
        
        // 创建测量线
        m_measureLine = new QGraphicsLineItem();
        m_measureLine->setPen(QPen(Qt::red, 2, Qt::DashLine));
        m_graphicsScene->addItem(m_measureLine);
        
        // 创建测量文本
        m_measureText = new QGraphicsTextItem();
        QFont font;
        font.setPointSize(10);
        font.setBold(true);
        m_measureText->setFont(font);
        m_measureText->setDefaultTextColor(Qt::red);
        m_graphicsScene->addItem(m_measureText);
        
        drawMeasurementLine();
    } else {
        if (!m_isMeasureCompleted) {
            // 第二次点击：固定终点，测量完成
            QPointF endPos = scenePos;
            
            // 如果按下了Shift键，应用水平/竖直约束
            if (m_isShiftPressed) {
                // 计算水平和竖直距离
                double dx = qAbs(scenePos.x() - m_measureStart.x());
                double dy = qAbs(scenePos.y() - m_measureStart.y());
                
                // 根据距离选择水平或竖直约束
                if (dx > dy) {
                    // 水平约束，保持y坐标不变
                    endPos.setY(m_measureStart.y());
                } else {
                    // 竖直约束，保持x坐标不变
                    endPos.setX(m_measureStart.x());
                }
            }
            
            m_measureEnd = endPos;
            m_isMeasureCompleted = true;
            drawMeasurementLine();
        } else {
            // 第三次点击：开始新的测量，清除旧的测量线
            clearMeasurement();
        }
    }
}

void MainWindow::handleMouseMove(QPointF scenePos)
{
    if (!m_isMeasureMode || m_measureLine == nullptr || m_isMeasureCompleted) {
        return;
    }
    
    QPointF endPos = scenePos;
    
    // 如果按下了Shift键，约束测量线为水平或竖直
    if (m_isShiftPressed) {
        // 计算水平和竖直距离
        double dx = qAbs(scenePos.x() - m_measureStart.x());
        double dy = qAbs(scenePos.y() - m_measureStart.y());
        
        // 根据距离选择水平或竖直约束
        if (dx > dy) {
            // 水平约束，保持y坐标不变
            endPos.setY(m_measureStart.y());
        } else {
            // 竖直约束，保持x坐标不变
            endPos.setX(m_measureStart.x());
        }
    }
    
    // 更新终点
    m_measureEnd = endPos;
    drawMeasurementLine();
}

void MainWindow::drawMeasurementLine()
{
    if (m_measureLine == nullptr || m_measureText == nullptr) {
        return;
    }
    
    // 获取当前缩放因子
    qreal scaleFactor = m_graphicsView->transform().m11();
    
    // 防止缩放因子为0
    if (scaleFactor <= 0) {
        scaleFactor = 1.0;
    }
    
    // 动态计算视觉参数，保持视觉上的适当比例
    double lineWidth = 2.0 / scaleFactor;
    double fontSize = 10.0 / scaleFactor;
    double textOffset = 25.0 / scaleFactor;
    
    // 设置最小值限制，避免线条过细或文字过小
    lineWidth = qMax(lineWidth, 1.0);
    fontSize = qMax(fontSize, 6.0);
    textOffset = qMax(textOffset, 15.0);
    
    // 更新测量线的线条宽度
    m_measureLine->setPen(QPen(Qt::red, lineWidth, Qt::DashLine));
    
    // 更新测量线
    m_measureLine->setLine(m_measureStart.x(), m_measureStart.y(), m_measureEnd.x(), m_measureEnd.y());
    
    // 计算距离
    double distance = calculateDistance(m_measureStart, m_measureEnd);
    
    // 更新测量文本 - 使用Qt的arg语法，%1作为占位符，0表示宽度，'f'表示浮点数，2表示小数位数
    QString text = QString("距离: %1 像素").arg(distance, 0, 'f', 2);
    m_measureText->setPlainText(text);
    
    // 更新字体大小
    QFont font;
    font.setPointSize(fontSize);
    font.setBold(true);
    m_measureText->setFont(font);
    
    // 设置文本位置（位于线段中点，距离线段上方动态偏移）
    QPointF midPoint(
        (m_measureStart.x() + m_measureEnd.x()) / 2,
        (m_measureStart.y() + m_measureEnd.y()) / 2 - textOffset
    );
    m_measureText->setPos(midPoint);
}

void MainWindow::clearMeasurement()
{
    if (m_measureLine != nullptr) {
        m_graphicsScene->removeItem(m_measureLine);
        delete m_measureLine;
        m_measureLine = nullptr;
    }
    
    if (m_measureText != nullptr) {
        m_graphicsScene->removeItem(m_measureText);
        delete m_measureText;
        m_measureText = nullptr;
    }
    
    // 重置测量状态
    m_measureStart = QPointF();
    m_measureEnd = QPointF();
    m_isMeasureCompleted = false;
}

double MainWindow::calculateDistance(const QPointF &p1, const QPointF &p2)
{
    // 使用欧几里得距离公式计算两点间距离
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    return sqrt(dx * dx + dy * dy);
}

void MainWindow::updateMeasurementScale()
{
    if (m_measureLine == nullptr || m_measureText == nullptr) {
        return;
    }
    
    drawMeasurementLine();
}
