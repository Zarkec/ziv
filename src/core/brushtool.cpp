#include "brushtool.h"
#include "utils/panelstyle.h"
#include "core/imagegraphicsview.h"
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPen>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QColorDialog>
#include <QPainter>
#include <QPixmap>
#include <QCursor>

BrushTool::BrushTool(QGraphicsScene *scene, QGraphicsView *view, QObject *parent)
    : QObject(parent)
    , m_scene(scene)
    , m_view(view)
    , m_isBrushMode(false)
    , m_isDrawing(false)
    , m_currentPath(nullptr)
    , m_currentPainterPath(nullptr)
    , m_brushColor(Qt::red)
    , m_brushSize(20)
    , m_brushOpacity(100)
    , m_infoPanel(nullptr)
    , m_colorPreview(nullptr)
    , m_colorButton(nullptr)
    , m_sizeSlider(nullptr)
    , m_sizeSpinBox(nullptr)
    , m_opacitySlider(nullptr)
    , m_opacitySpinBox(nullptr)
    , m_isDarkTheme(false)
{
    createInfoPanel();
}

BrushTool::~BrushTool()
{
    if (m_infoPanel) {
        delete m_infoPanel;
    }
    if (m_currentPainterPath) {
        delete m_currentPainterPath;
    }
}

void BrushTool::toggleBrushMode(bool enabled)
{
    m_isBrushMode = enabled;
    
    ImageGraphicsView* graphicsView = qobject_cast<ImageGraphicsView*>(m_view);
    
    if (!m_isBrushMode) {
        m_view->setCursor(Qt::ArrowCursor);
        m_view->setDragMode(QGraphicsView::ScrollHandDrag);
        if (graphicsView) {
            graphicsView->clearBrushPreview();
        }
        m_isDrawing = false;
        m_currentPath = nullptr;
        if (m_currentPainterPath) {
            delete m_currentPainterPath;
            m_currentPainterPath = nullptr;
        }
    } else {
        m_view->setCursor(Qt::CrossCursor);
        m_view->setDragMode(QGraphicsView::NoDrag);
        updateBrushPreview();
    }
    
    emit modeChanged(enabled);
}

void BrushTool::handleMousePress(QPointF scenePos)
{
    if (!m_isBrushMode) return;
    
    m_isDrawing = true;
    m_lastPoint = scenePos;
    
    m_currentPainterPath = new QPainterPath();
    m_currentPainterPath->moveTo(scenePos);
    m_currentPainterPath->lineTo(scenePos + QPointF(0.01, 0.01));
    
    m_currentPath = new QGraphicsPathItem();
    QColor penColor = m_brushColor;
    penColor.setAlpha(m_brushOpacity * 255 / 100);
    
    QPen pen(penColor, m_brushSize, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    m_currentPath->setPen(pen);
    m_currentPath->setPath(*m_currentPainterPath);
    m_scene->addItem(m_currentPath);
}

void BrushTool::handleMouseMove(QPointF scenePos)
{
    if (!m_isBrushMode || !m_isDrawing || !m_currentPath || !m_currentPainterPath) return;
    
    m_currentPainterPath->lineTo(scenePos);
    m_currentPath->setPath(*m_currentPainterPath);
    m_lastPoint = scenePos;
}

void BrushTool::handleMouseRelease(QPointF scenePos)
{
    Q_UNUSED(scenePos);
    
    if (!m_isBrushMode || !m_isDrawing) return;
    
    m_isDrawing = false;
    
    if (m_currentPath) {
        m_brushStrokes.append(m_currentPath);
        m_undoStack.push(m_currentPath);
        while (!m_redoStack.isEmpty()) {
            m_redoStack.pop();
        }
        m_currentPath = nullptr;
    }
    
    if (m_currentPainterPath) {
        delete m_currentPainterPath;
        m_currentPainterPath = nullptr;
    }
}

bool BrushTool::isBrushMode() const
{
    return m_isBrushMode;
}

QWidget* BrushTool::getInfoPanel() const
{
    return m_infoPanel;
}

void BrushTool::updateTheme(bool isDarkTheme)
{
    m_isDarkTheme = isDarkTheme;
    if (m_infoPanel) {
        PanelStyle::instance().applyPanelStyle(m_infoPanel, isDarkTheme);
        
        QList<QFrame*> frames = m_infoPanel->findChildren<QFrame*>();
        for (QFrame* frame : frames) {
            if (frame->frameShape() == QFrame::HLine) {
                frame->setStyleSheet(PanelStyle::instance().getSeparatorStyleSheet(isDarkTheme));
            }
        }
        
        QString borderStyle = isDarkTheme ? 
            QString("background-color: %1; border: 2px solid #ffffff; border-radius: 5px;").arg(m_brushColor.name()) :
            QString("background-color: %1; border: 2px solid #333333; border-radius: 5px;").arg(m_brushColor.name());
        m_colorPreview->setStyleSheet(borderStyle);
    }
}

void BrushTool::clearBrushStrokes()
{
    for (QGraphicsPathItem* item : m_brushStrokes) {
        m_scene->removeItem(item);
        delete item;
    }
    m_brushStrokes.clear();
    m_undoStack.clear();
    m_redoStack.clear();
    
    if (m_currentPath) {
        m_scene->removeItem(m_currentPath);
        delete m_currentPath;
        m_currentPath = nullptr;
    }
    
    if (m_currentPainterPath) {
        delete m_currentPainterPath;
        m_currentPainterPath = nullptr;
    }
}

void BrushTool::setBrushColor(const QColor &color)
{
    m_brushColor = color;
    
    QString borderStyle = m_isDarkTheme ? 
        QString("background-color: %1; border: 2px solid #ffffff; border-radius: 5px;").arg(color.name()) :
        QString("background-color: %1; border: 2px solid #333333; border-radius: 5px;").arg(color.name());
    m_colorPreview->setStyleSheet(borderStyle);
    
    if (m_isBrushMode) {
        updateBrushPreview();
    }
}

QColor BrushTool::brushColor() const
{
    return m_brushColor;
}

void BrushTool::setBrushSize(int size)
{
    m_brushSize = size;
    if (m_sizeSlider) {
        m_sizeSlider->blockSignals(true);
        m_sizeSlider->setValue(size);
        m_sizeSlider->blockSignals(false);
    }
    if (m_sizeSpinBox) {
        m_sizeSpinBox->blockSignals(true);
        m_sizeSpinBox->setValue(size);
        m_sizeSpinBox->blockSignals(false);
    }
    if (m_isBrushMode) {
        updateBrushPreview();
    }
}

int BrushTool::brushSize() const
{
    return m_brushSize;
}

void BrushTool::setBrushOpacity(int opacity)
{
    m_brushOpacity = opacity;
}

int BrushTool::brushOpacity() const
{
    return m_brushOpacity;
}

void BrushTool::onScaleChanged()
{
    if (m_isBrushMode) {
        updateBrushPreview();
    }
}

void BrushTool::undo()
{
    if (m_undoStack.isEmpty()) return;
    
    QGraphicsPathItem* item = m_undoStack.pop();
    m_scene->removeItem(item);
    m_brushStrokes.removeOne(item);
    m_redoStack.push(item);
}

void BrushTool::redo()
{
    if (m_redoStack.isEmpty()) return;
    
    QGraphicsPathItem* item = m_redoStack.pop();
    m_scene->addItem(item);
    m_brushStrokes.append(item);
    m_undoStack.push(item);
}

bool BrushTool::canUndo() const
{
    return !m_undoStack.isEmpty();
}

bool BrushTool::canRedo() const
{
    return !m_redoStack.isEmpty();
}

void BrushTool::createInfoPanel()
{
    m_infoPanel = new QWidget();
    m_infoPanel->setObjectName("toolPanel");
    QVBoxLayout *layout = new QVBoxLayout(m_infoPanel);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(6);
    
    PanelStyle& style = PanelStyle::instance();
    
    QLabel *titleLabel = style.createTitleLabel("画笔工具", m_isDarkTheme);
    layout->addWidget(titleLabel);
    
    QFrame *line0 = style.createSeparator(m_isDarkTheme);
    layout->addWidget(line0);
    
    QLabel *colorTitle = style.createSectionLabel("颜色", m_isDarkTheme);
    layout->addWidget(colorTitle);
    
    QHBoxLayout *colorLayout = new QHBoxLayout();
    m_colorPreview = new QLabel();
    m_colorPreview->setFixedSize(40, 40);
    QString borderStyle = m_isDarkTheme ? 
        QString("background-color: %1; border: 2px solid #ffffff; border-radius: 5px;").arg(m_brushColor.name()) :
        QString("background-color: %1; border: 2px solid #333333; border-radius: 5px;").arg(m_brushColor.name());
    m_colorPreview->setStyleSheet(borderStyle);
    colorLayout->addWidget(m_colorPreview);
    
    m_colorButton = new QPushButton("选择颜色");
    connect(m_colorButton, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(m_brushColor, nullptr, "选择画笔颜色");
        if (color.isValid()) {
            setBrushColor(color);
        }
    });
    colorLayout->addWidget(m_colorButton);
    colorLayout->addStretch();
    layout->addLayout(colorLayout);
    
    QFrame *line1 = style.createSeparator(m_isDarkTheme);
    layout->addWidget(line1);
    
    QLabel *sizeTitle = style.createSectionLabel("大小", m_isDarkTheme);
    layout->addWidget(sizeTitle);
    
    QHBoxLayout *sizeLayout = new QHBoxLayout();
    m_sizeSlider = new QSlider(Qt::Horizontal);
    m_sizeSlider->setMinimum(1);
    m_sizeSlider->setMaximum(500);
    m_sizeSlider->setValue(m_brushSize);
    m_sizeSpinBox = new QSpinBox();
    m_sizeSpinBox->setMinimum(1);
    m_sizeSpinBox->setMaximum(500);
    m_sizeSpinBox->setValue(m_brushSize);
    m_sizeSpinBox->setSuffix(" px");
    m_sizeSpinBox->setFixedWidth(60);
    sizeLayout->addWidget(m_sizeSlider, 1);
    sizeLayout->addWidget(m_sizeSpinBox);
    layout->addLayout(sizeLayout);
    
    connect(m_sizeSlider, &QSlider::valueChanged, this, [this](int value) {
        m_sizeSpinBox->blockSignals(true);
        m_sizeSpinBox->setValue(value);
        m_sizeSpinBox->blockSignals(false);
        setBrushSize(value);
    });
    connect(m_sizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        m_sizeSlider->blockSignals(true);
        m_sizeSlider->setValue(value);
        m_sizeSlider->blockSignals(false);
        setBrushSize(value);
    });
    
    QFrame *line2 = style.createSeparator(m_isDarkTheme);
    layout->addWidget(line2);
    
    QLabel *opacityTitle = style.createSectionLabel("透明度", m_isDarkTheme);
    layout->addWidget(opacityTitle);
    
    QHBoxLayout *opacityLayout = new QHBoxLayout();
    m_opacitySlider = new QSlider(Qt::Horizontal);
    m_opacitySlider->setMinimum(1);
    m_opacitySlider->setMaximum(100);
    m_opacitySlider->setValue(m_brushOpacity);
    m_opacitySpinBox = new QSpinBox();
    m_opacitySpinBox->setMinimum(1);
    m_opacitySpinBox->setMaximum(100);
    m_opacitySpinBox->setValue(m_brushOpacity);
    m_opacitySpinBox->setSuffix("%");
    m_opacitySpinBox->setFixedWidth(60);
    opacityLayout->addWidget(m_opacitySlider, 1);
    opacityLayout->addWidget(m_opacitySpinBox);
    layout->addLayout(opacityLayout);
    
    connect(m_opacitySlider, &QSlider::valueChanged, this, [this](int value) {
        m_opacitySpinBox->blockSignals(true);
        m_opacitySpinBox->setValue(value);
        m_opacitySpinBox->blockSignals(false);
        setBrushOpacity(value);
    });
    connect(m_opacitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        m_opacitySlider->blockSignals(true);
        m_opacitySlider->setValue(value);
        m_opacitySlider->blockSignals(false);
        setBrushOpacity(value);
    });
    
    QFrame *line3 = style.createSeparator(m_isDarkTheme);
    layout->addWidget(line3);
    
    QPushButton *clearButton = new QPushButton("清除画笔");
    connect(clearButton, &QPushButton::clicked, this, &BrushTool::clearBrushStrokes);
    layout->addWidget(clearButton);
    
    layout->addStretch();
    
    style.applyPanelStyle(m_infoPanel, m_isDarkTheme);
}

void BrushTool::updateBrushPreview()
{
    if (!m_isBrushMode) return;
    
    ImageGraphicsView* graphicsView = qobject_cast<ImageGraphicsView*>(m_view);
    if (graphicsView) {
        graphicsView->setBrushPreview(m_brushColor, m_brushSize, true);
    }
}
