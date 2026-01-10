#include "measurementtool.h"
#include <QFont>
#include <QPen>
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
{
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
        m_measureLine->setPen(QPen(Qt::red, 2, Qt::DashLine));
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
    
    m_measureLine->setPen(QPen(Qt::red, lineWidth, Qt::DashLine));
    
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
}

double MeasurementTool::calculateDistance(const QPointF &p1, const QPointF &p2)
{
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    return sqrt(dx * dx + dy * dy);
}
