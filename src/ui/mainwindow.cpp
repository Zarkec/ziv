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
#include <QIcon>
#include <QHBoxLayout>
#include <QProgressDialog>
#include <QThread>
#include <QtConcurrent>
#include <QCoreApplication>
#include <QMap>
#include <QGuiApplication>
#include <QStyleHints>

#include "core/imagegraphicsview.h"
#include "core/imageviewer.h"
#include "core/measurementtool.h"
#include "core/anglemeasurementtool.h"

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
    , m_isDarkTheme(false)
    , m_overlayModeAction(nullptr)
    , m_overlayControlPanel(nullptr)
    , m_image2PathLabel(nullptr)
    , m_loadImage2Button(nullptr)
    , m_clearImage2Button(nullptr)
    , m_alpha1Label(nullptr)
    , m_alpha1Slider(nullptr)
    , m_alpha1SpinBox(nullptr)
    , m_alpha2Label(nullptr)
    , m_alpha2Slider(nullptr)
    , m_alpha2SpinBox(nullptr)
{
    m_isDarkTheme = isSystemDarkTheme();

    setAcceptDrops(true);

    setupUI();
    setupActions();
    setupConnections();

    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &MainWindow::onPaletteChanged);

    m_graphicsView->setEnabled(false);
}

MainWindow::~MainWindow() 
{
    delete m_imageViewer;
    delete m_measurementTool;
}

void MainWindow::openFile(const QString &fileName)
{
    if (!fileName.isEmpty()) {
        m_imageViewer->openImage(fileName);
    }
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
    m_imageSizeLabel = new QLabel("图片大小: 0 B", this);
    m_imageIndexLabel = new QLabel("0/0", this);
    m_loadingLabel = new QLabel("", this);
    
    statusBar()->addWidget(m_coordinateLabel);
    statusBar()->addPermanentWidget(m_loadingLabel);
    statusBar()->addPermanentWidget(m_imageIndexLabel);
    statusBar()->addPermanentWidget(m_imageSizeLabel);
    statusBar()->addPermanentWidget(m_sizeLabel);
    statusBar()->addPermanentWidget(m_scaleLabel);
    
    m_zoomSlider = new QSlider(Qt::Horizontal, this);
    m_zoomSlider->setMinimum(1);
    m_zoomSlider->setMaximum(3200);
    m_zoomSlider->setValue(100);
    m_zoomSlider->setEnabled(false);
    m_zoomSlider->setFixedWidth(150);
    statusBar()->addPermanentWidget(m_zoomSlider);
    
    m_zoomSpinBox = new QSpinBox(this);
    m_zoomSpinBox->setMinimum(1);
    m_zoomSpinBox->setMaximum(3200);
    m_zoomSpinBox->setValue(100);
    m_zoomSpinBox->setSuffix("%");
    m_zoomSpinBox->setFixedWidth(80);
    m_zoomSpinBox->setEnabled(false);
    statusBar()->addPermanentWidget(m_zoomSpinBox);
    
    resize(1000, 650);
    setMinimumSize(550, 400);
    setWindowTitle("图片查看器");
    setWindowIcon(QIcon(":/icons/icon.png"));
    
    m_imageViewer = new ImageViewer(m_graphicsView, m_graphicsScene, this);
    m_imageViewer->setCoordinateLabel(m_coordinateLabel);
    m_imageViewer->setScaleLabel(m_scaleLabel);
    m_imageViewer->setSizeLabel(m_sizeLabel);
    m_imageViewer->setImageSizeLabel(m_imageSizeLabel);
    m_imageViewer->setZoomSlider(m_zoomSlider);
    m_imageViewer->setZoomSpinBox(m_zoomSpinBox);
    m_imageViewer->setImageIndexLabel(m_imageIndexLabel);
    
    m_measurementTool = new MeasurementTool(m_graphicsScene, m_graphicsView, this);
    m_angleMeasurementTool = new AngleMeasurementTool(m_graphicsScene, m_graphicsView, this);

    createOverlayControlPanel();
}

void MainWindow::setupActions()
{
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    QMenu *viewMenu = menuBar()->addMenu("视图(&V)");
    
    QAction *openAction = new QAction("打开", this);
    openAction->setIcon(QIcon(":/icons/light/open.png"));
    openAction->setShortcut(QKeySequence::Open);
    fileMenu->addAction(openAction);
    m_iconActions["open"] = openAction;
    
    QAction *exportAction = new QAction("导出", this);
    exportAction->setIcon(QIcon(":/icons/light/export.png"));
    exportAction->setShortcut(QKeySequence::Save);
    fileMenu->addAction(exportAction);
    m_iconActions["export"] = exportAction;
    
    QAction *zoomInAction = new QAction("放大", this);
    zoomInAction->setIcon(QIcon(":/icons/light/zoom_in.png"));
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    m_iconActions["zoom_in"] = zoomInAction;
    
    QAction *zoomOutAction = new QAction("缩小", this);
    zoomOutAction->setIcon(QIcon(":/icons/light/zoom_out.png"));
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    m_iconActions["zoom_out"] = zoomOutAction;
    
    m_fitToWindowAction = new QAction("适应窗口", this);
    m_fitToWindowAction->setIcon(QIcon(":/icons/light/fit_to_window.png"));
    m_fitToWindowAction->setCheckable(true);
    m_iconActions["fit_to_window"] = m_fitToWindowAction;
    
    QAction *originalSizeAction = new QAction("原始大小", this);
    originalSizeAction->setIcon(QIcon(":/icons/light/original_size.png"));
    originalSizeAction->setShortcut(tr("Ctrl+1"));
    m_iconActions["original_size"] = originalSizeAction;
    
    QAction *rotateLeftAction = new QAction("向左旋转", this);
    rotateLeftAction->setIcon(QIcon(":/icons/light/rotate_left.png"));
    rotateLeftAction->setShortcut(tr("Ctrl+L"));
    m_iconActions["rotate_left"] = rotateLeftAction;
    
    QAction *rotateRightAction = new QAction("向右旋转", this);
    rotateRightAction->setIcon(QIcon(":/icons/light/rotate_right.png"));
    rotateRightAction->setShortcut(tr("Ctrl+R"));
    m_iconActions["rotate_right"] = rotateRightAction;
    
    QAction *rotate180Action = new QAction("旋转180度", this);
    rotate180Action->setIcon(QIcon(":/icons/light/rotate_180.png"));
    rotate180Action->setShortcut(tr("Ctrl+O"));
    m_iconActions["rotate_180"] = rotate180Action;
    
    QAction *flipHorizontalAction = new QAction("水平翻转", this);
    flipHorizontalAction->setIcon(QIcon(":/icons/light/flip_horizontal.png"));
    flipHorizontalAction->setShortcut(tr("Ctrl+H"));
    m_iconActions["flip_horizontal"] = flipHorizontalAction;
    
    QAction *flipVerticalAction = new QAction("垂直翻转", this);
    flipVerticalAction->setIcon(QIcon(":/icons/light/flip_vertical.png"));
    flipVerticalAction->setShortcut(tr("Ctrl+V"));
    m_iconActions["flip_vertical"] = flipVerticalAction;
    
    m_measureAction = new QAction("测量模式", this);
    m_measureAction->setIcon(QIcon(":/icons/light/measure.png"));
    m_measureAction->setCheckable(true);
    m_measureAction->setShortcut(tr("Ctrl+M"));
    m_iconActions["measure"] = m_measureAction;
    
    m_angleAction = new QAction("测角模式", this);
    m_angleAction->setIcon(QIcon(":/icons/light/angle.png"));
    m_angleAction->setCheckable(true);
    m_angleAction->setShortcut(tr("Ctrl+A"));
    m_iconActions["angle"] = m_angleAction;
    
    QAction *previousImageAction = new QAction("上一张", this);
    previousImageAction->setIcon(QIcon(":/icons/light/previous.png"));
    previousImageAction->setShortcut(Qt::Key_Left);
    m_iconActions["previous"] = previousImageAction;
    
    QAction *nextImageAction = new QAction("下一张", this);
    nextImageAction->setIcon(QIcon(":/icons/light/next.png"));
    nextImageAction->setShortcut(Qt::Key_Right);
    m_iconActions["next"] = nextImageAction;

m_overlayModeAction = new QAction("叠加模式", this);
    m_overlayModeAction->setIcon(QIcon(":/icons/light/overlay.png"));
    m_overlayModeAction->setCheckable(true);
    m_overlayModeAction->setShortcut(tr("Ctrl+Y"));
    m_iconActions["overlay"] = m_overlayModeAction;

    viewMenu->addAction(zoomInAction);
    viewMenu->addAction(zoomOutAction);
    viewMenu->addSeparator();
    viewMenu->addAction(previousImageAction);
    viewMenu->addAction(nextImageAction);
    viewMenu->addSeparator();
    viewMenu->addAction(rotateLeftAction);
    viewMenu->addAction(rotateRightAction);
    viewMenu->addAction(rotate180Action);
    viewMenu->addSeparator();
    viewMenu->addAction(flipHorizontalAction);
    viewMenu->addAction(flipVerticalAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_measureAction);
    viewMenu->addAction(m_angleAction);
viewMenu->addAction(m_overlayModeAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_fitToWindowAction);
    viewMenu->addAction(originalSizeAction);

    QToolBar *toolBar = new QToolBar("查看工具栏", this);
    addToolBar(Qt::LeftToolBarArea, toolBar);
    toolBar->setAllowedAreas(Qt::LeftToolBarArea | Qt::RightToolBarArea);
    toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
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
    toolBar->addAction(previousImageAction);
    toolBar->addAction(nextImageAction);
    toolBar->addSeparator();
    toolBar->addAction(m_measureAction);
toolBar->addAction(m_angleAction);
    toolBar->addAction(m_overlayModeAction);
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
    connect(previousImageAction, &QAction::triggered, this, &MainWindow::previousImage);
    connect(nextImageAction, &QAction::triggered, this, &MainWindow::nextImage);
    connect(m_measureAction, &QAction::triggered, this, &MainWindow::toggleMeasureMode);
    connect(m_angleAction, &QAction::triggered, this, &MainWindow::toggleAngleMode);
    connect(m_fitToWindowAction, &QAction::triggered, this, &MainWindow::fitToWindow);
    connect(originalSizeAction, &QAction::triggered, this, &MainWindow::originalSize);
connect(m_overlayModeAction, &QAction::triggered, this, &MainWindow::toggleOverlayMode);

    updateThemeIcons();
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
            m_angleMeasurementTool->handleMouseMove(scenePos);
        }
    });
    
    connect(m_graphicsView, &ImageGraphicsView::mousePressed, this, [this](QPointF scenePos) {
        if (m_imageViewer->pixmapItem() && m_graphicsScene->sceneRect().contains(scenePos)) {
            m_measurementTool->handleMousePress(scenePos);
            m_angleMeasurementTool->handleMousePress(scenePos);
        }
    });
    
    connect(m_imageViewer, &ImageViewer::fitToWindowChanged, this, [this](bool fit) {
        m_fitToWindowAction->setChecked(fit);
    });
    
    connect(m_graphicsView, &ImageGraphicsView::shiftPressed, this, [this]() {
        m_measurementTool->setShiftPressed(true);
        m_angleMeasurementTool->setShiftPressed(true);
        if (m_measurementTool->isMeasureMode()) {
            QPoint cursorPos = m_graphicsView->mapFromGlobal(QCursor::pos());
            QPointF scenePos = m_graphicsView->mapToScene(cursorPos);
            m_measurementTool->handleMouseMove(scenePos);
        }
        if (m_angleMeasurementTool->isAngleMode()) {
            QPoint cursorPos = m_graphicsView->mapFromGlobal(QCursor::pos());
            QPointF scenePos = m_graphicsView->mapToScene(cursorPos);
            m_angleMeasurementTool->handleMouseMove(scenePos);
        }
    });
    
    connect(m_graphicsView, &ImageGraphicsView::shiftReleased, this, [this]() {
        m_measurementTool->setShiftPressed(false);
        m_angleMeasurementTool->setShiftPressed(false);
        if (m_measurementTool->isMeasureMode()) {
            QPoint cursorPos = m_graphicsView->mapFromGlobal(QCursor::pos());
            QPointF scenePos = m_graphicsView->mapToScene(cursorPos);
            m_measurementTool->handleMouseMove(scenePos);
        }
        if (m_angleMeasurementTool->isAngleMode()) {
            QPoint cursorPos = m_graphicsView->mapFromGlobal(QCursor::pos());
            QPointF scenePos = m_graphicsView->mapToScene(cursorPos);
            m_angleMeasurementTool->handleMouseMove(scenePos);
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
        m_angleMeasurementTool->updateMeasurementScale();
    });
    
    connect(m_imageViewer, &ImageViewer::imageLoaded, this, [this](const QString &fileName) {
        m_measurementTool->clearMeasurement();
        m_angleMeasurementTool->clearMeasurement();
        setWindowTitle(tr("图片查看器 - %1").arg(fileName));
    });
    
    connect(m_imageViewer, &ImageViewer::scaleChanged, this, [this]() {
        m_measurementTool->clearMeasurement();
        m_angleMeasurementTool->clearMeasurement();
    });
    
    connect(m_imageViewer, &ImageViewer::imageLoadingStarted, this, [this]() {
        m_loadingLabel->setText(tr("加载中..."));
        m_loadingLabel->setStyleSheet("color: #FFA500;");
    });
    
    connect(m_imageViewer, &ImageViewer::imageLoadingFinished, this, [this]() {
        m_loadingLabel->setText("");
    });

    // Overlay mode connections
    connect(m_imageViewer, &ImageViewer::overlayModeChanged, m_overlayControlPanel, &QWidget::setVisible);
    connect(m_imageViewer, &ImageViewer::secondImageLoaded, this, [this](const QString &fileName) {
        QFileInfo fi(fileName);
        m_image2PathLabel->setText(fi.fileName());
        m_clearImage2Button->setEnabled(true);
    });
    connect(m_imageViewer, &ImageViewer::secondImageCleared, this, [this]() {
        m_image2PathLabel->setText("未加载");
        m_clearImage2Button->setEnabled(false);
    });

    connect(m_loadImage2Button, &QPushButton::clicked, this, &MainWindow::loadSecondImage);
    connect(m_clearImage2Button, &QPushButton::clicked, this, &MainWindow::clearSecondImage);

    connect(m_alpha1Slider, &QSlider::valueChanged, this, &MainWindow::onAlpha1Changed);
    connect(m_alpha2Slider, &QSlider::valueChanged, this, &MainWindow::onAlpha2Changed);
    connect(m_alpha1SpinBox, QOverload<int>::of(&QSpinBox::valueChanged), m_alpha1Slider, &QSlider::setValue);
    connect(m_alpha2SpinBox, QOverload<int>::of(&QSpinBox::valueChanged), m_alpha2Slider, &QSlider::setValue);
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
    if (m_measureAction->isChecked()) {
        m_angleAction->setChecked(false);
        m_angleMeasurementTool->toggleAngleMode(false);
    }
    m_measurementTool->toggleMeasureMode(m_measureAction->isChecked());
}

void MainWindow::toggleAngleMode()
{
    if (m_angleAction->isChecked()) {
        m_measureAction->setChecked(false);
        m_measurementTool->toggleMeasureMode(false);
    }
    m_angleMeasurementTool->toggleAngleMode(m_angleAction->isChecked());
}

void MainWindow::nextImage()
{
    m_imageViewer->nextImage();
}

void MainWindow::previousImage()
{
    m_imageViewer->previousImage();
}

void MainWindow::onPaletteChanged()
{
    m_isDarkTheme = isSystemDarkTheme();
    updateThemeIcons();
}

bool MainWindow::isSystemDarkTheme()
{
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}

void MainWindow::updateThemeIcons()
{
    QString themePath = m_isDarkTheme ? ":/icons/dark/" : ":/icons/light/";
    
    for (auto it = m_iconActions.begin(); it != m_iconActions.end(); ++it) {
        QString iconName = it.key();
        QAction *action = it.value();
        action->setIcon(QIcon(themePath + iconName + ".png"));
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    m_imageViewer->resizeEvent();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        if (!urls.isEmpty()) {
            QString fileName = urls.first().toLocalFile();
            openFile(fileName);
        }
    }

    event->acceptProposedAction();
}

void MainWindow::createOverlayControlPanel()
{
    m_overlayControlPanel = new QWidget(this);

    QHBoxLayout *panelLayout = new QHBoxLayout(m_overlayControlPanel);
    panelLayout->setContentsMargins(10, 5, 10, 5);

    // Image 2 loading section
    QLabel *image2Label = new QLabel("图片2:", this);
    m_image2PathLabel = new QLabel("未加载", this);
    m_image2PathLabel->setMinimumWidth(150);
    m_loadImage2Button = new QPushButton("加载", this);
    m_clearImage2Button = new QPushButton("清除", this);
    m_clearImage2Button->setEnabled(false);

    panelLayout->addWidget(image2Label);
    panelLayout->addWidget(m_image2PathLabel);
    panelLayout->addWidget(m_loadImage2Button);
    panelLayout->addWidget(m_clearImage2Button);

    panelLayout->addSpacing(20);

    // Alpha 1 control section
    m_alpha1Label = new QLabel("图片1透明度:", this);
    m_alpha1Slider = new QSlider(Qt::Horizontal, this);
    m_alpha1Slider->setMinimum(0);
    m_alpha1Slider->setMaximum(100);
    m_alpha1Slider->setValue(50);
    m_alpha1Slider->setMinimumWidth(150);
    m_alpha1SpinBox = new QSpinBox(this);
    m_alpha1SpinBox->setMinimum(0);
    m_alpha1SpinBox->setMaximum(100);
    m_alpha1SpinBox->setValue(50);
    m_alpha1SpinBox->setSuffix("%");

    panelLayout->addWidget(m_alpha1Label);
    panelLayout->addWidget(m_alpha1Slider);
    panelLayout->addWidget(m_alpha1SpinBox);

    panelLayout->addSpacing(20);

    // Alpha 2 control section
    m_alpha2Label = new QLabel("图片2透明度:", this);
    m_alpha2Slider = new QSlider(Qt::Horizontal, this);
    m_alpha2Slider->setMinimum(0);
    m_alpha2Slider->setMaximum(100);
    m_alpha2Slider->setValue(50);
    m_alpha2Slider->setMinimumWidth(150);
    m_alpha2SpinBox = new QSpinBox(this);
    m_alpha2SpinBox->setMinimum(0);
    m_alpha2SpinBox->setMaximum(100);
    m_alpha2SpinBox->setValue(50);
    m_alpha2SpinBox->setSuffix("%");

    panelLayout->addWidget(m_alpha2Label);
    panelLayout->addWidget(m_alpha2Slider);
    panelLayout->addWidget(m_alpha2SpinBox);

    panelLayout->addStretch();

    // Add panel to main layout (above graphics view)
    QWidget *centralWidget = this->centralWidget();
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(centralWidget->layout());
    if (mainLayout) {
        mainLayout->insertWidget(0, m_overlayControlPanel);
    }

    m_overlayControlPanel->setVisible(false);
}

void MainWindow::toggleOverlayMode()
{
    if (!m_imageViewer->isEnabled()) {
        m_overlayModeAction->setChecked(false);
        QMessageBox::warning(this, tr("警告"), tr("请先打开一张图片"));
        return;
    }

    bool enabled = m_overlayModeAction->isChecked();
    m_imageViewer->enableOverlayMode(enabled);

    if (enabled) {
        // Disable measurement tools when overlay mode is active
        m_measureAction->setChecked(false);
        m_measurementTool->toggleMeasureMode(false);
        m_angleAction->setChecked(false);
        m_angleMeasurementTool->toggleAngleMode(false);
    }
}

void MainWindow::loadSecondImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("打开第二张图片"),
        QString(),
        tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.tiff *.tif *.webp);;所有文件 (*.*)")
    );

    if (!fileName.isEmpty()) {
        m_imageViewer->loadSecondImage(fileName);
    }
}

void MainWindow::clearSecondImage()
{
    m_imageViewer->clearSecondImage();
}

void MainWindow::onAlpha1Changed(int value)
{
    m_alpha1SpinBox->blockSignals(true);
    m_alpha1SpinBox->setValue(value);
    m_alpha1SpinBox->blockSignals(false);

    m_imageViewer->setAlpha1(value / 100.0);
}

void MainWindow::onAlpha2Changed(int value)
{
    m_alpha2SpinBox->blockSignals(true);
    m_alpha2SpinBox->setValue(value);
    m_alpha2SpinBox->blockSignals(false);

    m_imageViewer->setAlpha2(value / 100.0);
}
