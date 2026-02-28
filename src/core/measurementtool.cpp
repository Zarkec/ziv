#include "measurementtool.h"
#include "utils/panelstyle.h"
#include <QFont>
#include <QPen>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <cmath>

MeasurementTool::MeasurementTool(QGraphicsScene *scene, QGraphicsView *view, QObject *parent)
    : QObject(parent)
    , m_scene(scene)
    , m_view(view)
    , m_measureLine(nullptr)
    , m_measureText(nullptr)
    , m_isMeasureMode(false)
    , m_isMeasureCompleted(false)
    , m_isShiftPressed(false)
    , m_infoPanel(nullptr)
    , m_distanceLabel(nullptr)
    , m_startPosLabel(nullptr)
    , m_endPosLabel(nullptr)
    , m_deltaLabel(nullptr)
    , m_isDarkTheme(false)
{
    createInfoPanel();
}

void MeasurementTool::toggleMeasureMode(bool enabled)
{
    m_isMeasureMode = enabled;
    
    if (!m_isMeasureMode) {
        clearMeasurement();
        m_view->setCursor(Qt::ArrowCursor);
        m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    } else {
        m_view->setCursor(Qt::CrossCursor);
        m_view->setDragMode(QGraphicsView::NoDrag);
    }
    
    emit modeChanged(enabled);
}

void MeasurementTool::handleMousePress(QPointF scenePos)
{
    if (!m_isMeasureMode) {
        return;
    }
    
    if (m_measureLine == nullptr) {
        m_measureStart = scenePos;
        m_measureEnd = scenePos;
        m_isMeasureCompleted = false;
        
        m_measureLine = new QGraphicsLineItem();
        m_measureLine->setPen(QPen(Qt::red, 2, Qt::SolidLine));
        m_scene->addItem(m_measureLine);
        
        m_measureText = new QGraphicsTextItem();
        QFont font;
        font.setPointSize(10);
        font.setBold(true);
        m_measureText->setFont(font);
        m_measureText->setDefaultTextColor(Qt::red);
        m_scene->addItem(m_measureText);
        
        drawMeasurementLine();
    } else {
        if (!m_isMeasureCompleted) {
            QPointF endPos = scenePos;
            
            if (m_isShiftPressed) {
                double dx = qAbs(scenePos.x() - m_measureStart.x());
                double dy = qAbs(scenePos.y() - m_measureStart.y());
                
                if (dx > dy) {
                    endPos.setY(m_measureStart.y());
                } else {
                    endPos.setX(m_measureStart.x());
                }
            }
            
            m_measureEnd = endPos;
            m_isMeasureCompleted = true;
            drawMeasurementLine();
        } else {
            clearMeasurement();
        }
    }
}

void MeasurementTool::handleMouseMove(QPointF scenePos)
{
    if (!m_isMeasureMode || m_measureLine == nullptr || m_isMeasureCompleted) {
        return;
    }
    
    QPointF endPos = scenePos;
    
    if (m_isShiftPressed) {
        double dx = qAbs(scenePos.x() - m_measureStart.x());
        double dy = qAbs(scenePos.y() - m_measureStart.y());
        
        if (dx > dy) {
            endPos.setY(m_measureStart.y());
        } else {
            endPos.setX(m_measureStart.x());
        }
    }
    
    m_measureEnd = endPos;
    drawMeasurementLine();
}

void MeasurementTool::setShiftPressed(bool pressed)
{
    m_isShiftPressed = pressed;
}

void MeasurementTool::clearMeasurement()
{
    if (m_measureLine != nullptr) {
        m_scene->removeItem(m_measureLine);
        delete m_measureLine;
        m_measureLine = nullptr;
    }
    
    if (m_measureText != nullptr) {
        m_scene->removeItem(m_measureText);
        delete m_measureText;
        m_measureText = nullptr;
    }
    
    m_measureStart = QPointF();
    m_measureEnd = QPointF();
    m_isMeasureCompleted = false;
    
    // 重置信息面板
    updateInfoPanel(0, QPointF(), QPointF());
}

void MeasurementTool::updateMeasurementScale()
{
    if (m_measureLine == nullptr || m_measureText == nullptr) {
        return;
    }
    
    drawMeasurementLine();
}

bool MeasurementTool::isMeasureMode() const
{
    return m_isMeasureMode;
}

void MeasurementTool::drawMeasurementLine()
{
    if (m_measureLine == nullptr || m_measureText == nullptr) {
        return;
    }
    
    qreal scaleFactor = m_view->transform().m11();
    
    if (scaleFactor <= 0) {
        scaleFactor = 1.0;
    }
    
    double lineWidth = 2.0 / scaleFactor;
    double fontSize = 10.0 / scaleFactor;
    double textOffset = 25.0 / scaleFactor;
    
    lineWidth = qMax(lineWidth, 1.0);
    fontSize = qMax(fontSize, 6.0);
    textOffset = qMax(textOffset, 15.0);
    
    m_measureLine->setPen(QPen(Qt::red, lineWidth, Qt::SolidLine));
    
    m_measureLine->setLine(m_measureStart.x(), m_measureStart.y(), m_measureEnd.x(), m_measureEnd.y());
    
    double distance = calculateDistance(m_measureStart, m_measureEnd);
    
    QString text = QString("距离: %1 像素").arg(distance, 0, 'f', 2);
    m_measureText->setPlainText(text);
    
    QFont font;
    font.setPointSize(fontSize);
    font.setBold(true);
    m_measureText->setFont(font);
    
    QPointF midPoint(
        (m_measureStart.x() + m_measureEnd.x()) / 2,
        (m_measureStart.y() + m_measureEnd.y()) / 2 - textOffset
    );
    m_measureText->setPos(midPoint);
    
    // 更新信息面板
    updateInfoPanel(distance, m_measureStart, m_measureEnd);
}

double MeasurementTool::calculateDistance(const QPointF &p1, const QPointF &p2)
{
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    return sqrt(dx * dx + dy * dy);
}

QWidget* MeasurementTool::getInfoPanel() const
{
    return m_infoPanel;
}

void MeasurementTool::updateTheme(bool isDarkTheme)
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
    }
}

void MeasurementTool::createInfoPanel()
{
    m_infoPanel = new QWidget();
    m_infoPanel->setObjectName("toolPanel");
    QVBoxLayout *layout = new QVBoxLayout(m_infoPanel);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(6);
    
    PanelStyle& style = PanelStyle::instance();
    
    QLabel *titleLabel = style.createTitleLabel("测量工具", m_isDarkTheme);
    layout->addWidget(titleLabel);
    
    QFrame *line0 = style.createSeparator(m_isDarkTheme);
    layout->addWidget(line0);
    
    QLabel *distanceTitle = style.createSectionLabel("距离", m_isDarkTheme);
    layout->addWidget(distanceTitle);
    
    m_distanceLabel = style.createEmphasisLabel("0.00 像素", m_isDarkTheme);
    layout->addWidget(m_distanceLabel);
    
    QFrame *line1 = style.createSeparator(m_isDarkTheme);
    layout->addWidget(line1);
    
    QLabel *startTitle = style.createSectionLabel("起点", m_isDarkTheme);
    layout->addWidget(startTitle);
    
    m_startPosLabel = style.createContentLabel("(0.0, 0.0)", m_isDarkTheme);
    layout->addWidget(m_startPosLabel);
    
    QFrame *line2 = style.createSeparator(m_isDarkTheme);
    layout->addWidget(line2);
    
    QLabel *endTitle = style.createSectionLabel("终点", m_isDarkTheme);
    layout->addWidget(endTitle);
    
    m_endPosLabel = style.createContentLabel("(0.0, 0.0)", m_isDarkTheme);
    layout->addWidget(m_endPosLabel);
    
    QFrame *line3 = style.createSeparator(m_isDarkTheme);
    layout->addWidget(line3);
    
    QLabel *deltaTitle = style.createSectionLabel("增量", m_isDarkTheme);
    layout->addWidget(deltaTitle);
    
    m_deltaLabel = style.createContentLabel("Δx=0.0, Δy=0.0", m_isDarkTheme);
    layout->addWidget(m_deltaLabel);
    
    layout->addStretch();
    
    style.applyPanelStyle(m_infoPanel, m_isDarkTheme);
}

void MeasurementTool::updateInfoPanel(double distance, const QPointF &start, const QPointF &end)
{
    if (m_distanceLabel) {
        m_distanceLabel->setText(QString("%1 像素").arg(distance, 0, 'f', 2));
    }
    if (m_startPosLabel) {
        m_startPosLabel->setText(QString("(%1, %2)").arg(start.x(), 0, 'f', 1).arg(start.y(), 0, 'f', 1));
    }
    if (m_endPosLabel) {
        m_endPosLabel->setText(QString("(%1, %2)").arg(end.x(), 0, 'f', 1).arg(end.y(), 0, 'f', 1));
    }
    if (m_deltaLabel) {
        double dx = end.x() - start.x();
        double dy = end.y() - start.y();
        m_deltaLabel->setText(QString("Δx=%1, Δy=%2").arg(dx, 0, 'f', 1).arg(dy, 0, 'f', 1));
    }
}
