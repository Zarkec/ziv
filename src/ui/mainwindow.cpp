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
#include <QShortcut>
#include <QKeySequence>
#include <QStyleHints>
#include <QStackedWidget>
#include <QGroupBox>
#include <QFrame>

#include "core/imagegraphicsview.h"
#include "core/imageviewer.h"
#include "core/measurementtool.h"
#include "core/anglemeasurementtool.h"
#include "core/colorpickertool.h"
#include "core/brushtool.h"
#include "utils/panelstyle.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_graphicsView(nullptr)
    , m_graphicsScene(nullptr)
    , m_coordinateIconLabel(nullptr)
    , m_coordinateLabel(nullptr)
    , m_scaleLabel(nullptr)
    , m_sizeLabel(nullptr)
    , m_fitToWindowAction(nullptr)
    , m_measureAction(nullptr)
    , m_angleAction(nullptr)
    , m_colorPickerAction(nullptr)
    , m_brushAction(nullptr)
    , m_imageViewer(nullptr)
    , m_measurementTool(nullptr)
    , m_colorPickerTool(nullptr)
    , m_brushTool(nullptr)
    , m_zoomSlider(nullptr)
    , m_zoomSpinBox(nullptr)
    , m_isDarkTheme(false)
    , m_overlayModeAction(nullptr)
    , m_overlayControlPanel(nullptr)
    , m_toolsBarDock(nullptr)
    , m_toolsToolBar(nullptr)
    , m_toolsDock(nullptr)
    , m_toolsPanel(nullptr)
    , m_toolsStack(nullptr)
    , m_image2PathLabel(nullptr)
    , m_loadImage2Button(nullptr)
    , m_clearImage2Button(nullptr)
    , m_alpha1Label(nullptr)
    , m_alpha1Slider(nullptr)
    , m_alpha1SpinBox(nullptr)
    , m_alpha2Label(nullptr)
    , m_alpha2Slider(nullptr)
    , m_alpha2SpinBox(nullptr)
    , m_overlaySettingsPanel(nullptr)
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
    delete m_angleMeasurementTool;
    delete m_colorPickerTool;
    delete m_brushTool;
}

void MainWindow::openFile(const QString &fileName)
{
    if (!fileName.isEmpty()) {
        m_imageViewer->openImage(fileName);
    }
}

void MainWindow::setupUI()
{
    // 创建中央部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    
    // 创建图形场景和视图
    m_graphicsScene = new QGraphicsScene(this);
    m_graphicsView = new ImageGraphicsView(this);
    m_graphicsView->setScene(m_graphicsScene);
    
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    m_graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);
    m_graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    m_graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    m_graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    
    centralLayout->addWidget(m_graphicsView);
    
    // 创建工具
    m_measurementTool = new MeasurementTool(m_graphicsScene, m_graphicsView, this);
    m_angleMeasurementTool = new AngleMeasurementTool(m_graphicsScene, m_graphicsView, this);
    m_colorPickerTool = new ColorPickerTool(m_graphicsScene, m_graphicsView, this);
    m_brushTool = new BrushTool(m_graphicsScene, m_graphicsView, this);
    m_colorPickerTool->updateTheme(m_isDarkTheme);
    m_measurementTool->updateTheme(m_isDarkTheme);
    m_angleMeasurementTool->updateTheme(m_isDarkTheme);
    m_brushTool->updateTheme(m_isDarkTheme);
    
    // 创建右侧工具栏 DockWidget (放在上方)
    m_toolsBarDock = new QDockWidget("", this);
    m_toolsBarDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_toolsBarDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    m_toolsBarDock->setMinimumWidth(280);
    m_toolsBarDock->setMaximumWidth(350);
    m_toolsBarDock->setTitleBarWidget(new QWidget()); // 隐藏标题栏
    
    // 创建工具栏
    m_toolsToolBar = new QToolBar("工具栏", this);
    m_toolsToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_toolsToolBar->setMovable(false);
    m_toolsToolBar->setFloatable(false);
    
    // 设置工具栏样式
    m_toolsToolBar->setStyleSheet(
        "QToolBar { background-color: transparent; border: none; padding: 4px; }"
        "QToolButton { border: none; padding: 6px 12px; margin: 2px; border-radius: 4px; }"
        "QToolButton:hover { background-color: rgba(128, 128, 128, 0.2); }"
        "QToolButton:checked { background-color: rgba(0, 120, 215, 0.3); }"
    );
    
    QWidget *toolBarContainer = new QWidget();
    QHBoxLayout *toolBarLayout = new QHBoxLayout(toolBarContainer);
    toolBarLayout->setContentsMargins(0, 0, 0, 0);
    toolBarLayout->setSpacing(0);
    toolBarLayout->addWidget(m_toolsToolBar);
    toolBarLayout->addStretch();
    
    m_toolsBarDock->setWidget(toolBarContainer);
    addDockWidget(Qt::RightDockWidgetArea, m_toolsBarDock);
    
    // 创建右侧工具面板 DockWidget (放在下方)
    m_toolsDock = new QDockWidget("工具面板", this);
    m_toolsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_toolsDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    m_toolsDock->setMinimumWidth(280);
    m_toolsDock->setMaximumWidth(350);
    
    // 工具面板内容
    m_toolsPanel = new QWidget();
    QVBoxLayout *toolsLayout = new QVBoxLayout(m_toolsPanel);
    toolsLayout->setContentsMargins(0, 0, 0, 0);
    toolsLayout->setSpacing(0);
    
    // 使用堆叠窗口显示不同工具的面板
    m_toolsStack = new QStackedWidget();
    
    // 添加测量工具面板
    m_toolsStack->addWidget(m_measurementTool->getInfoPanel());
    // 添加测角工具面板
    m_toolsStack->addWidget(m_angleMeasurementTool->getInfoPanel());
    // 添加取色器面板
    m_toolsStack->addWidget(m_colorPickerTool->getColorInfoPanel());
    // 添加画笔工具面板
    m_toolsStack->addWidget(m_brushTool->getInfoPanel());
    
    // 创建叠加控制面板（也放在工具面板中）
    createOverlayControlPanel();
    m_toolsStack->addWidget(m_overlaySettingsPanel);
    
    toolsLayout->addWidget(m_toolsStack);
    toolsLayout->addStretch();
    
    m_toolsDock->setWidget(m_toolsPanel);
    addDockWidget(Qt::RightDockWidgetArea, m_toolsDock);
    
    // 将工具栏停靠在工具面板上方
    splitDockWidget(m_toolsBarDock, m_toolsDock, Qt::Vertical);
    
    // 工具面板和工具栏默认始终显示
    m_toolsDock->setVisible(true);
    m_toolsBarDock->setVisible(true);
    
    // 默认显示测量工具面板（但不激活测量模式）
    m_toolsStack->setCurrentIndex(0);
    
    // 状态栏
    m_coordinateIconLabel = new QLabel(this);
    m_coordinateIconLabel->setFixedSize(20, 20);
    m_coordinateIconLabel->setScaledContents(true);
    QString coordIconPath = m_isDarkTheme ? ":/icons/dark/coordinate.png" : ":/icons/light/coordinate.png";
    m_coordinateIconLabel->setPixmap(QIcon(coordIconPath).pixmap(20, 20));
    
    m_coordinateLabel = new QLabel("坐标: (0, 0)", this);
    m_scaleLabel = new QLabel("缩放:", this);
    m_sizeLabel = new QLabel("尺寸: 0x0", this);
    m_imageSizeLabel = new QLabel("图片大小: 0 B", this);
    m_imageIndexLabel = new QLabel("0/0", this);
    m_loadingLabel = new QLabel("", this);
    
    statusBar()->addWidget(m_coordinateIconLabel);
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
    
    resize(1200, 650);
    setMinimumSize(900, 500);
    setWindowTitle("ziv");
    setWindowIcon(QIcon(":/icons/icon.png"));
    
    m_imageViewer = new ImageViewer(m_graphicsView, m_graphicsScene, this);
    m_imageViewer->setCoordinateLabel(m_coordinateLabel);
    m_imageViewer->setScaleLabel(m_scaleLabel);
    m_imageViewer->setSizeLabel(m_sizeLabel);
    m_imageViewer->setImageSizeLabel(m_imageSizeLabel);
    m_imageViewer->setZoomSlider(m_zoomSlider);
    m_imageViewer->setZoomSpinBox(m_zoomSpinBox);
    m_imageViewer->setImageIndexLabel(m_imageIndexLabel);
}

void MainWindow::setupActions()
{
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    QMenu *viewMenu = menuBar()->addMenu("视图(&V)");
    QMenu *toolsMenu = menuBar()->addMenu("工具(&T)");
    
    // === 文件操作 ===
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
    
    fileMenu->addSeparator();
    
    // === 视图操作 ===
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
    
    QAction *previousImageAction = new QAction("上一张", this);
    previousImageAction->setIcon(QIcon(":/icons/light/previous.png"));
    previousImageAction->setShortcut(Qt::Key_Left);
    m_iconActions["previous"] = previousImageAction;
    
    QAction *nextImageAction = new QAction("下一张", this);
    nextImageAction->setIcon(QIcon(":/icons/light/next.png"));
    nextImageAction->setShortcut(Qt::Key_Right);
    m_iconActions["next"] = nextImageAction;
    
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
    viewMenu->addAction(m_fitToWindowAction);
    viewMenu->addAction(originalSizeAction);
    
    // === 工具操作 ===
    m_measureAction = new QAction("测量", this);
    m_measureAction->setIcon(QIcon(":/icons/light/measure.png"));
    m_measureAction->setCheckable(true);
    m_measureAction->setShortcut(tr("Ctrl+M"));
    m_iconActions["measure"] = m_measureAction;
    
    m_angleAction = new QAction("测角", this);
    m_angleAction->setIcon(QIcon(":/icons/light/angle.png"));
    m_angleAction->setCheckable(true);
    m_angleAction->setShortcut(tr("Ctrl+A"));
    m_iconActions["angle"] = m_angleAction;
    
    m_colorPickerAction = new QAction("取色", this);
    m_colorPickerAction->setIcon(QIcon(":/icons/light/colorpicker.png"));
    m_colorPickerAction->setCheckable(true);
    m_colorPickerAction->setShortcut(tr("Ctrl+P"));
    m_iconActions["colorpicker"] = m_colorPickerAction;
    
    m_overlayModeAction = new QAction("叠加", this);
    m_overlayModeAction->setIcon(QIcon(":/icons/light/overlay.png"));
    m_overlayModeAction->setCheckable(true);
    m_overlayModeAction->setShortcut(tr("Ctrl+Y"));
    m_iconActions["overlay"] = m_overlayModeAction;
    
    m_brushAction = new QAction("画笔", this);
    m_brushAction->setIcon(QIcon(":/icons/light/brush.png"));
    m_brushAction->setCheckable(true);
    m_brushAction->setShortcut(tr("Ctrl+B"));
    m_iconActions["brush"] = m_brushAction;
    
    toolsMenu->addAction(m_measureAction);
    toolsMenu->addAction(m_angleAction);
    toolsMenu->addAction(m_colorPickerAction);
    toolsMenu->addAction(m_brushAction);
    toolsMenu->addAction(m_overlayModeAction);
    
    // === 左侧工具栏 (文件和视图操作) ===
    QToolBar *leftToolBar = new QToolBar("查看工具栏", this);
    addToolBar(Qt::LeftToolBarArea, leftToolBar);
    leftToolBar->setAllowedAreas(Qt::LeftToolBarArea | Qt::RightToolBarArea);
    leftToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    leftToolBar->addAction(openAction);
    leftToolBar->addAction(exportAction);
    leftToolBar->addSeparator();
    leftToolBar->addAction(zoomInAction);
    leftToolBar->addAction(zoomOutAction);
    leftToolBar->addSeparator();
    leftToolBar->addAction(rotateLeftAction);
    leftToolBar->addAction(rotateRightAction);
    leftToolBar->addAction(rotate180Action);
    leftToolBar->addSeparator();
    leftToolBar->addAction(flipHorizontalAction);
    leftToolBar->addAction(flipVerticalAction);
    leftToolBar->addSeparator();
    leftToolBar->addAction(previousImageAction);
    leftToolBar->addAction(nextImageAction);
    leftToolBar->addSeparator();
    leftToolBar->addAction(m_fitToWindowAction);
    leftToolBar->addAction(originalSizeAction);
    
    // === 工具面板内的工具栏（添加到m_toolsToolBar） ===
    m_toolsToolBar->addAction(m_measureAction);
    m_toolsToolBar->addAction(m_angleAction);
    m_toolsToolBar->addAction(m_colorPickerAction);
    m_toolsToolBar->addAction(m_brushAction);
    m_toolsToolBar->addAction(m_overlayModeAction);
    
    // 连接信号
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
    connect(m_colorPickerAction, &QAction::triggered, this, &MainWindow::toggleColorPickerMode);
    connect(m_brushAction, &QAction::triggered, this, &MainWindow::toggleBrushMode);
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
            m_colorPickerTool->handleMouseMove(scenePos);
            m_brushTool->handleMouseMove(scenePos);
        }
    });
    
    connect(m_graphicsView, &ImageGraphicsView::mousePressed, this, [this](QPointF scenePos) {
        if (m_imageViewer->pixmapItem() && m_graphicsScene->sceneRect().contains(scenePos)) {
            m_measurementTool->handleMousePress(scenePos);
            m_angleMeasurementTool->handleMousePress(scenePos);
            m_colorPickerTool->handleMousePress(scenePos);
            m_brushTool->handleMousePress(scenePos);
        }
    });
    
    connect(m_graphicsView, &ImageGraphicsView::mouseReleased, this, [this](QPointF scenePos) {
        m_brushTool->handleMouseRelease(scenePos);
    });

    connect(m_colorPickerTool, &ColorPickerTool::selectionModeChanged, this, [this](bool enabled, QPointF position) {
        if (enabled) {
            m_graphicsView->setFixedCrosshairPosition(position);
        } else {
            m_graphicsView->clearFixedCrosshair();
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
        m_brushTool->onScaleChanged();
    });
    
    connect(m_graphicsView, &ImageGraphicsView::brushSizeAdjustRequested, this, [this](int delta) {
        if (m_brushTool->isBrushMode()) {
            int currentSize = m_brushTool->brushSize();
            int newSize = currentSize + (delta > 0 ? 5 : -5);
            newSize = qBound(1, newSize, 500);
            m_brushTool->setBrushSize(newSize);
        }
    });
    
    connect(m_imageViewer, &ImageViewer::imageLoaded, this, [this](const QString &fileName) {
        m_measurementTool->clearMeasurement();
        m_angleMeasurementTool->clearMeasurement();
        // 更新取色器的图像数据
        if (m_imageViewer->pixmapItem()) {
            m_colorPickerTool->setImage(m_imageViewer->originalPixmap().toImage());
        }
        setWindowTitle(tr("ziv - %1").arg(fileName));
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
    
    QShortcut *undoShortcut = new QShortcut(QKeySequence::Undo, this);
    connect(undoShortcut, &QShortcut::activated, this, [this]() {
        if (m_brushTool->canUndo()) {
            m_brushTool->undo();
        }
    });
    
    QShortcut *redoShortcut = new QShortcut(QKeySequence::Redo, this);
    connect(redoShortcut, &QShortcut::activated, this, [this]() {
        if (m_brushTool->canRedo()) {
            m_brushTool->redo();
        }
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

void MainWindow::updateToolsPanel(int toolIndex)
{
    m_toolsStack->setCurrentIndex(toolIndex);
    m_toolsDock->setVisible(true);
    m_toolsBarDock->setVisible(true);
    m_toolsDock->raise();
}

void MainWindow::toggleMeasureMode()
{
    if (!m_imageViewer->isEnabled()) {
        m_measureAction->setChecked(false);
        QMessageBox::warning(this, tr("警告"), tr("请先打开一张图片"));
        return;
    }
    
    if (m_measureAction->isChecked()) {
        m_angleAction->setChecked(false);
        m_angleMeasurementTool->toggleAngleMode(false);
        m_colorPickerAction->setChecked(false);
        m_colorPickerTool->toggleColorPickerMode(false);
        m_brushAction->setChecked(false);
        m_brushTool->toggleBrushMode(false);
        m_overlayModeAction->setChecked(false);
        m_imageViewer->enableOverlayMode(false);
        updateToolsPanel(0);  // 显示测量面板
    }
    // 右侧面板始终显示，不隐藏
    m_measurementTool->toggleMeasureMode(m_measureAction->isChecked());
}

void MainWindow::toggleAngleMode()
{
    if (!m_imageViewer->isEnabled()) {
        m_angleAction->setChecked(false);
        QMessageBox::warning(this, tr("警告"), tr("请先打开一张图片"));
        return;
    }
    
    if (m_angleAction->isChecked()) {
        m_measureAction->setChecked(false);
        m_measurementTool->toggleMeasureMode(false);
        m_colorPickerAction->setChecked(false);
        m_colorPickerTool->toggleColorPickerMode(false);
        m_brushAction->setChecked(false);
        m_brushTool->toggleBrushMode(false);
        m_overlayModeAction->setChecked(false);
        m_imageViewer->enableOverlayMode(false);
        updateToolsPanel(1);  // 显示测角面板
    }
    // 右侧面板始终显示，不隐藏
    m_angleMeasurementTool->toggleAngleMode(m_angleAction->isChecked());
}

void MainWindow::toggleColorPickerMode()
{
    if (!m_imageViewer->isEnabled()) {
        m_colorPickerAction->setChecked(false);
        QMessageBox::warning(this, tr("警告"), tr("请先打开一张图片"));
        return;
    }

    if (m_colorPickerAction->isChecked()) {
        m_measureAction->setChecked(false);
        m_measurementTool->toggleMeasureMode(false);
        m_angleAction->setChecked(false);
        m_angleMeasurementTool->toggleAngleMode(false);
        m_brushAction->setChecked(false);
        m_brushTool->toggleBrushMode(false);
        m_overlayModeAction->setChecked(false);
        m_imageViewer->enableOverlayMode(false);

        m_colorPickerTool->setImage(m_imageViewer->originalPixmap().toImage());
        updateToolsPanel(2);  // 显示取色器面板
    }
    // 右侧面板始终显示，不隐藏

    m_colorPickerTool->toggleColorPickerMode(m_colorPickerAction->isChecked());
}

void MainWindow::toggleBrushMode()
{
    if (!m_imageViewer->isEnabled()) {
        m_brushAction->setChecked(false);
        QMessageBox::warning(this, tr("警告"), tr("请先打开一张图片"));
        return;
    }

    if (m_brushAction->isChecked()) {
        m_measureAction->setChecked(false);
        m_measurementTool->toggleMeasureMode(false);
        m_angleAction->setChecked(false);
        m_angleMeasurementTool->toggleAngleMode(false);
        m_colorPickerAction->setChecked(false);
        m_colorPickerTool->toggleColorPickerMode(false);
        m_overlayModeAction->setChecked(false);
        m_imageViewer->enableOverlayMode(false);
        updateToolsPanel(3);  // 显示画笔面板
    }

    m_brushTool->toggleBrushMode(m_brushAction->isChecked());
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
    m_colorPickerTool->updateTheme(m_isDarkTheme);
    m_measurementTool->updateTheme(m_isDarkTheme);
    m_angleMeasurementTool->updateTheme(m_isDarkTheme);
    m_brushTool->updateTheme(m_isDarkTheme);
    updateOverlayPanelTheme();
    
    QString coordIconPath = m_isDarkTheme ? ":/icons/dark/coordinate.png" : ":/icons/light/coordinate.png";
    m_coordinateIconLabel->setPixmap(QIcon(coordIconPath).pixmap(20, 20));
    
    QString panelStyle = m_isDarkTheme ?
        "QDockWidget { background-color: #1e1e1e; color: #e0e0e0; } "
        "QDockWidget::title { background-color: #2d2d2d; padding: 6px; } "
        "QWidget { background-color: #1e1e1e; color: #e0e0e0; }" :
        "QDockWidget { background-color: #f5f5f5; color: #333333; } "
        "QDockWidget::title { background-color: #e0e0e0; padding: 6px; } "
        "QWidget { background-color: #f5f5f5; color: #333333; }";
    m_toolsDock->setStyleSheet(panelStyle);
    m_toolsBarDock->setStyleSheet(panelStyle);
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
    m_overlaySettingsPanel = new QWidget();
    m_overlaySettingsPanel->setObjectName("toolPanel");
    QVBoxLayout *layout = new QVBoxLayout(m_overlaySettingsPanel);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(6);
    
    PanelStyle& style = PanelStyle::instance();
    
    QLabel *titleLabel = style.createTitleLabel("叠加模式", m_isDarkTheme);
    layout->addWidget(titleLabel);
    
    QFrame *line0 = style.createSeparator(m_isDarkTheme);
    layout->addWidget(line0);
    
    QLabel *image2Title = style.createSectionLabel("第二张图片", m_isDarkTheme);
    layout->addWidget(image2Title);
    
    QHBoxLayout *pathLayout = new QHBoxLayout();
    QLabel *image2Label = style.createEmphasisLabel("文件:", m_isDarkTheme);
    m_image2PathLabel = style.createContentLabel("未加载", m_isDarkTheme);
    pathLayout->addWidget(image2Label);
    pathLayout->addWidget(m_image2PathLabel, 1);
    layout->addLayout(pathLayout);
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_loadImage2Button = new QPushButton("加载");
    m_clearImage2Button = new QPushButton("清除");
    m_clearImage2Button->setEnabled(false);
    btnLayout->addWidget(m_loadImage2Button);
    btnLayout->addWidget(m_clearImage2Button);
    layout->addLayout(btnLayout);
    
    QFrame *line1 = style.createSeparator(m_isDarkTheme);
    layout->addWidget(line1);
    
    QLabel *alphaTitle = style.createSectionLabel("透明度控制", m_isDarkTheme);
    layout->addWidget(alphaTitle);
    
    QHBoxLayout *alpha1Layout = new QHBoxLayout();
    m_alpha1Label = style.createContentLabel("图片1:", m_isDarkTheme);
    m_alpha1Slider = new QSlider(Qt::Horizontal);
    m_alpha1Slider->setMinimum(0);
    m_alpha1Slider->setMaximum(100);
    m_alpha1Slider->setValue(50);
    m_alpha1SpinBox = new QSpinBox();
    m_alpha1SpinBox->setMinimum(0);
    m_alpha1SpinBox->setMaximum(100);
    m_alpha1SpinBox->setValue(50);
    m_alpha1SpinBox->setSuffix("%");
    m_alpha1SpinBox->setFixedWidth(60);
    alpha1Layout->addWidget(m_alpha1Label);
    alpha1Layout->addWidget(m_alpha1Slider, 1);
    alpha1Layout->addWidget(m_alpha1SpinBox);
    layout->addLayout(alpha1Layout);

    QHBoxLayout *alpha2Layout = new QHBoxLayout();
    m_alpha2Label = style.createContentLabel("图片2:", m_isDarkTheme);
    m_alpha2Slider = new QSlider(Qt::Horizontal);
    m_alpha2Slider->setMinimum(0);
    m_alpha2Slider->setMaximum(100);
    m_alpha2Slider->setValue(50);
    m_alpha2SpinBox = new QSpinBox();
    m_alpha2SpinBox->setMinimum(0);
    m_alpha2SpinBox->setMaximum(100);
    m_alpha2SpinBox->setValue(50);
    m_alpha2SpinBox->setSuffix("%");
    m_alpha2SpinBox->setFixedWidth(60);
    alpha2Layout->addWidget(m_alpha2Label);
    alpha2Layout->addWidget(m_alpha2Slider, 1);
    alpha2Layout->addWidget(m_alpha2SpinBox);
    layout->addLayout(alpha2Layout);
    
    layout->addStretch();
    
    style.applyPanelStyle(m_overlaySettingsPanel, m_isDarkTheme);
}

void MainWindow::updateOverlayPanelTheme()
{
    if (m_overlaySettingsPanel) {
        PanelStyle::instance().applyPanelStyle(m_overlaySettingsPanel, m_isDarkTheme);
        
        QList<QFrame*> frames = m_overlaySettingsPanel->findChildren<QFrame*>();
        for (QFrame* frame : frames) {
            if (frame->frameShape() == QFrame::HLine) {
                frame->setStyleSheet(PanelStyle::instance().getSeparatorStyleSheet(m_isDarkTheme));
            }
        }
    }
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
        m_measureAction->setChecked(false);
        m_measurementTool->toggleMeasureMode(false);
        m_angleAction->setChecked(false);
        m_angleMeasurementTool->toggleAngleMode(false);
        m_colorPickerAction->setChecked(false);
        m_colorPickerTool->toggleColorPickerMode(false);
        m_brushAction->setChecked(false);
        m_brushTool->toggleBrushMode(false);
        updateToolsPanel(4);  // 显示叠加控制面板
    }
    // 右侧面板始终显示，不隐藏
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
