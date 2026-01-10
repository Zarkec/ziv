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
#include <QProgressDialog>
#include <QThread>
#include <QtConcurrent>
#include <QCoreApplication>

#include "core/imagegraphicsview.h"
#include "core/imageviewer.h"
#include "core/measurementtool.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_graphicsView(nullptr)
    , m_graphicsScene(nullptr)
    , m_coordinateLabel(nullptr)
    , m_scaleLabel(nullptr)
    , m_sizeLabel(nullptr)
    , m_fitToWindowAction(nullptr)
    , m_measureAction(nullptr)
    , m_imageViewer(nullptr)
    , m_measurementTool(nullptr)
    , m_zoomSlider(nullptr)
    , m_zoomSpinBox(nullptr)
{
    setupUI();
    setupActions();
    setupConnections();
    
    m_graphicsView->setEnabled(false);
}

MainWindow::~MainWindow() 
{
    delete m_imageViewer;
    delete m_measurementTool;
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    m_graphicsScene = new QGraphicsScene(this);
    m_graphicsView = new ImageGraphicsView(this);
    m_graphicsView->setScene(m_graphicsScene);
    
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    m_graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);
    m_graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    m_graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    m_graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    
    mainLayout->addWidget(m_graphicsView);
    
    m_coordinateLabel = new QLabel("坐标: (0, 0)", this);
    m_scaleLabel = new QLabel("缩放:", this);
    m_sizeLabel = new QLabel("尺寸: 0x0", this);
    
    statusBar()->addWidget(m_coordinateLabel);
    statusBar()->addPermanentWidget(m_sizeLabel);
    statusBar()->addPermanentWidget(m_scaleLabel);
    
    m_zoomSlider = new QSlider(Qt::Horizontal, this);
    m_zoomSlider->setMinimum(1);
    m_zoomSlider->setMaximum(10000);
    m_zoomSlider->setValue(100);
    m_zoomSlider->setEnabled(false);
    m_zoomSlider->setFixedWidth(150);
    statusBar()->addPermanentWidget(m_zoomSlider);
    
    m_zoomSpinBox = new QSpinBox(this);
    m_zoomSpinBox->setMinimum(1);
    m_zoomSpinBox->setMaximum(10000);
    m_zoomSpinBox->setValue(100);
    m_zoomSpinBox->setSuffix("%");
    m_zoomSpinBox->setFixedWidth(80);
    m_zoomSpinBox->setEnabled(false);
    statusBar()->addPermanentWidget(m_zoomSpinBox);
    
    resize(800, 600);
    setWindowTitle("图片查看器");
    
    m_imageViewer = new ImageViewer(m_graphicsView, m_graphicsScene, this);
    m_imageViewer->setCoordinateLabel(m_coordinateLabel);
    m_imageViewer->setScaleLabel(m_scaleLabel);
    m_imageViewer->setSizeLabel(m_sizeLabel);
    m_imageViewer->setZoomSlider(m_zoomSlider);
    m_imageViewer->setZoomSpinBox(m_zoomSpinBox);
    
    m_measurementTool = new MeasurementTool(m_graphicsScene, m_graphicsView, this);
}

void MainWindow::setupActions()
{
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    QMenu *viewMenu = menuBar()->addMenu("视图(&V)");
    
    QAction *openAction = new QAction("打开(&O)", this);
    openAction->setShortcut(QKeySequence::Open);
    fileMenu->addAction(openAction);
    
    QAction *exportAction = new QAction("导出(&E)", this);
    exportAction->setShortcut(QKeySequence::Save);
    fileMenu->addAction(exportAction);
    
    QAction *zoomInAction = new QAction("放大(&+)", this);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    
    QAction *zoomOutAction = new QAction("缩小(&-)", this);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    
    m_fitToWindowAction = new QAction("适应窗口(&W)", this);
    m_fitToWindowAction->setCheckable(true);
    
    QAction *originalSizeAction = new QAction("原始大小(&1)", this);
    originalSizeAction->setShortcut(tr("Ctrl+1"));
    
    QAction *rotateLeftAction = new QAction("向左旋转(&L)", this);
    rotateLeftAction->setShortcut(tr("Ctrl+L"));
    
    QAction *rotateRightAction = new QAction("向右旋转(&R)", this);
    rotateRightAction->setShortcut(tr("Ctrl+R"));
    
    QAction *rotate180Action = new QAction("旋转180度(&O)", this);
    rotate180Action->setShortcut(tr("Ctrl+O"));
    
    QAction *flipHorizontalAction = new QAction("水平翻转(&H)", this);
    flipHorizontalAction->setShortcut(tr("Ctrl+H"));
    
    QAction *flipVerticalAction = new QAction("垂直翻转(&V)", this);
    flipVerticalAction->setShortcut(tr("Ctrl+V"));
    
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
    viewMenu->addAction(flipHorizontalAction);
    viewMenu->addAction(flipVerticalAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_measureAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_fitToWindowAction);
    viewMenu->addAction(originalSizeAction);
    
    QToolBar *toolBar = addToolBar("查看工具栏");
    toolBar->addAction(openAction);
    toolBar->addAction(exportAction);
    toolBar->addSeparator();
    toolBar->addAction(zoomInAction);
    toolBar->addAction(zoomOutAction);
    toolBar->addSeparator();
    toolBar->addAction(rotateLeftAction);
    toolBar->addAction(rotateRightAction);
    toolBar->addAction(rotate180Action);
    toolBar->addSeparator();
    toolBar->addAction(flipHorizontalAction);
    toolBar->addAction(flipVerticalAction);
    toolBar->addSeparator();
    toolBar->addAction(m_measureAction);
    toolBar->addSeparator();
    toolBar->addAction(m_fitToWindowAction);
    toolBar->addAction(originalSizeAction);
    
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
    connect(exportAction, &QAction::triggered, this, &MainWindow::exportImage);
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(rotateLeftAction, &QAction::triggered, this, &MainWindow::rotateLeft);
    connect(rotateRightAction, &QAction::triggered, this, &MainWindow::rotateRight);
    connect(rotate180Action, &QAction::triggered, this, &MainWindow::rotate180);
    connect(flipHorizontalAction, &QAction::triggered, this, &MainWindow::flipHorizontal);
    connect(flipVerticalAction, &QAction::triggered, this, &MainWindow::flipVertical);
    connect(m_measureAction, &QAction::triggered, this, &MainWindow::toggleMeasureMode);
    connect(m_fitToWindowAction, &QAction::triggered, this, &MainWindow::fitToWindow);
    connect(originalSizeAction, &QAction::triggered, this, &MainWindow::originalSize);
}

void MainWindow::setupConnections()
{
    connect(m_zoomSlider, &QSlider::valueChanged, this, [this](int value) {
        m_zoomSpinBox->setValue(value);
        m_imageViewer->applyZoom(value);
    });
    
    connect(m_zoomSpinBox, &QSpinBox::valueChanged, this, [this](int value) {
        m_zoomSlider->setValue(value);
        m_imageViewer->applyZoom(value);
    });
    
    connect(m_graphicsView, &ImageGraphicsView::mouseMoved, this, [this](QPointF scenePos) {
        if (m_imageViewer->pixmapItem() && m_graphicsScene->sceneRect().contains(scenePos)) {
            m_imageViewer->updateCoordinates(scenePos);
            m_measurementTool->handleMouseMove(scenePos);
        }
    });
    
    connect(m_graphicsView, &ImageGraphicsView::mousePressed, this, [this](QPointF scenePos) {
        if (m_imageViewer->pixmapItem() && m_graphicsScene->sceneRect().contains(scenePos)) {
            m_measurementTool->handleMousePress(scenePos);
        }
    });
    
    connect(m_graphicsView, &ImageGraphicsView::shiftPressed, this, [this]() {
        m_measurementTool->setShiftPressed(true);
        if (m_measurementTool->isMeasureMode()) {
            QPoint cursorPos = m_graphicsView->mapFromGlobal(QCursor::pos());
            QPointF scenePos = m_graphicsView->mapToScene(cursorPos);
            m_measurementTool->handleMouseMove(scenePos);
        }
    });
    
    connect(m_graphicsView, &ImageGraphicsView::shiftReleased, this, [this]() {
        m_measurementTool->setShiftPressed(false);
        if (m_measurementTool->isMeasureMode()) {
            QPoint cursorPos = m_graphicsView->mapFromGlobal(QCursor::pos());
            QPointF scenePos = m_graphicsView->mapToScene(cursorPos);
            m_measurementTool->handleMouseMove(scenePos);
        }
    });
    
    connect(m_graphicsView, &ImageGraphicsView::mouseLeft, this, [this]() {
        m_coordinateLabel->setText("坐标: (0, 0)");
    });
    
    connect(m_graphicsView, &ImageGraphicsView::mouseEntered, this, [this](QPointF scenePos) {
        if (m_imageViewer->pixmapItem() && m_graphicsScene->sceneRect().contains(scenePos)) {
            m_imageViewer->updateCoordinates(scenePos);
        }
    });
    
    connect(m_graphicsView, &ImageGraphicsView::scaleChanged, this, [this]() {
        m_imageViewer->updateScaleInfo();
        m_measurementTool->updateMeasurementScale();
        
        if (m_imageViewer->isFitToWindow()) {
            m_imageViewer->setFitToWindow(false);
            m_fitToWindowAction->setChecked(false);
        }
    });
    
    connect(m_imageViewer, &ImageViewer::imageLoaded, this, [this](const QString &fileName) {
        m_measurementTool->clearMeasurement();
        setWindowTitle(tr("图片查看器 - %1").arg(fileName));
    });
    
    connect(m_imageViewer, &ImageViewer::scaleChanged, this, [this]() {
        m_measurementTool->clearMeasurement();
    });
}

void MainWindow::openImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, 
        tr("打开图片"), 
        QString(), 
        tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.tiff *.tif *.webp);;所有文件 (*.*)")
    );
    
    m_imageViewer->openImage(fileName);
}

void MainWindow::exportImage()
{
    if (!m_imageViewer->isEnabled() || !m_imageViewer->pixmapItem()) {
        QMessageBox::warning(this, tr("警告"), tr("没有可导出的图片"));
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("导出图片"),
        QString(),
        tr("PNG 图片 (*.png);;JPEG 图片 (*.jpg *.jpeg);;BMP 图片 (*.bmp);;TIFF 图片 (*.tiff *.tif);;WEBP 图片 (*.webp);;所有文件 (*.*)")
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QProgressDialog progressDialog(tr("正在保存图片..."), tr("取消"), 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setCancelButton(nullptr);
    progressDialog.setRange(0, 0);
    progressDialog.show();
    
    QFuture<bool> future = m_imageViewer->exportImageAsync(fileName);
    
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
        QThread::msleep(50);
    }
    
    progressDialog.close();
    
    if (future.result()) {
        QMessageBox::information(this, tr("成功"), tr("图片已成功导出到:\n%1").arg(fileName));
    } else {
        QMessageBox::warning(this, tr("错误"), tr("导出图片失败"));
    }
}

void MainWindow::zoomIn()
{
    m_imageViewer->zoomIn();
}

void MainWindow::zoomOut()
{
    m_imageViewer->zoomOut();
}

void MainWindow::fitToWindow()
{
    m_imageViewer->setFitToWindow(m_fitToWindowAction->isChecked());
    m_imageViewer->fitToWindow();
}

void MainWindow::originalSize()
{
    m_imageViewer->originalSize();
}

void MainWindow::rotateLeft()
{
    m_imageViewer->rotateLeft();
}

void MainWindow::rotateRight()
{
    m_imageViewer->rotateRight();
}

void MainWindow::rotate180()
{
    m_imageViewer->rotate180();
}

void MainWindow::flipHorizontal()
{
    m_imageViewer->flipHorizontal();
}

void MainWindow::flipVertical()
{
    m_imageViewer->flipVertical();
}

void MainWindow::toggleMeasureMode()
{
    m_measurementTool->toggleMeasureMode(m_measureAction->isChecked());
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    m_imageViewer->resizeEvent();
}
