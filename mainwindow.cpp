#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QMessageBox>
#include <QCursor>

// ImageGraphicsView 类实现
ImageGraphicsView::ImageGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{}

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
        
        // 执行缩放操作
        scale(scaleFactor, scaleFactor);
        
        // 发射缩放变化信号
        emit scaleChanged();
        
        // 阻止默认的滚动行为
        event->accept();
    } else {
        // 调用默认的滚轮事件处理（滚动）
        QGraphicsView::wheelEvent(event);
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
    , m_isFitToWindow(false)
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
    m_scaleLabel = new QLabel("缩放: 100%", this);
    m_sizeLabel = new QLabel("尺寸: 0x0", this);
    
    statusBar()->addPermanentWidget(m_sizeLabel);
    statusBar()->addPermanentWidget(m_scaleLabel);
    statusBar()->addPermanentWidget(m_coordinateLabel);
    
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
    
    viewMenu->addAction(zoomInAction);
    viewMenu->addAction(zoomOutAction);
    viewMenu->addSeparator();
    viewMenu->addAction(rotateLeftAction);
    viewMenu->addAction(rotateRightAction);
    viewMenu->addAction(rotate180Action);
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
    toolBar->addAction(m_fitToWindowAction);
    toolBar->addAction(originalSizeAction);
    
    // 连接信号槽
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(rotateLeftAction, &QAction::triggered, this, &MainWindow::rotateLeft);
    connect(rotateRightAction, &QAction::triggered, this, &MainWindow::rotateRight);
    connect(rotate180Action, &QAction::triggered, this, &MainWindow::rotate180);
    connect(m_fitToWindowAction, &QAction::triggered, this, &MainWindow::fitToWindow);
    connect(originalSizeAction, &QAction::triggered, this, &MainWindow::originalSize);
}

void MainWindow::setupConnections()
{
    // 连接自定义视图的信号
    connect(m_graphicsView, &ImageGraphicsView::mouseMoved, this, [this](QPointF scenePos) {
        // 确保坐标在图片范围内
        if (m_pixmapItem && m_graphicsScene->sceneRect().contains(scenePos)) {
            updateCoordinates(scenePos);
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
    
    // 使用QImageReader来处理图片
    QImageReader reader(fileName);
    if (!reader.canRead()) {
        QMessageBox::warning(this, tr("错误"), tr("无法读取图片文件: %1").arg(reader.errorString()));
        return;
    }
    
    QPixmap pixmap;
    QImage image;
    
    // 直接读取原始尺寸图片
    if (!reader.read(&image)) {
        QMessageBox::warning(this, tr("错误"), tr("读取图片失败: %1").arg(reader.errorString()));
        return;
    }
    
    // 转换为QPixmap
    pixmap = QPixmap::fromImage(image);
    m_originalPixmap = pixmap;
    
    // 获取图片信息
    QSize imageSize = image.size();
    QFile file(fileName);
    qint64 fileSize = file.size();
    
    // 如果已有图片项，先移除
    if (m_pixmapItem) {
        m_graphicsScene->removeItem(m_pixmapItem);
        delete m_pixmapItem;
        m_pixmapItem = nullptr;
    }
    
    // 创建新的图片项
    m_pixmapItem = m_graphicsScene->addPixmap(pixmap);
    m_graphicsScene->setSceneRect(pixmap.rect());
    
    // 启用图形视图
    m_graphicsView->setEnabled(true);
    
    // 重置视图
    originalSize();
    
    // 更新图片尺寸信息
    updateSizeInfo();
    
    // 更新窗口标题
    setWindowTitle(tr("图片查看器 - %1").arg(fileName));
}

void MainWindow::zoomIn()
{
    if (!m_graphicsView->isEnabled()) {
        return;
    }
    
    m_graphicsView->scale(1.2, 1.2);
    updateScaleInfo();
    
    // 如果处于适应窗口模式，退出该模式
    if (m_isFitToWindow) {
        m_isFitToWindow = false;
        m_fitToWindowAction->setChecked(false);
    }
}

void MainWindow::zoomOut()
{
    if (!m_graphicsView->isEnabled()) {
        return;
    }
    
    m_graphicsView->scale(1.0 / 1.2, 1.0 / 1.2);
    updateScaleInfo();
    
    // 如果处于适应窗口模式，退出该模式
    if (m_isFitToWindow) {
        m_isFitToWindow = false;
        m_fitToWindowAction->setChecked(false);
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

void MainWindow::updateScaleInfo()
{
    if (!m_graphicsView->isEnabled()) {
        m_scaleLabel->setText("缩放: 100%");
        return;
    }
    
    // 计算当前缩放比例
    qreal scale = m_graphicsView->transform().m11() * 100;
    m_scaleLabel->setText(tr("缩放: %1%").arg(qRound(scale)));
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
    
    // 如果处于适应窗口模式，重新调整大小
    if (m_isFitToWindow) {
        m_graphicsView->fitInView(m_pixmapItem, Qt::KeepAspectRatio);
    }
    
    // 更新尺寸信息和缩放信息
    updateSizeInfo();
    updateScaleInfo();
}
