#include "anglemeasurementtool.h"
#include "utils/panelstyle.h"
#include <QFont>
#include <QPen>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QFrame>
#include <cmath>

AngleMeasurementTool::AngleMeasurementTool(QGraphicsScene *scene, QGraphicsView *view, QObject *parent)
    : QObject(parent)
    , m_scene(scene)
    , m_view(view)
    , m_firstLine(nullptr)
    , m_secondLine(nullptr)
    , m_angleText(nullptr)
    , m_angleArc(nullptr)
    , m_clickCount(0)
    , m_isAngleMode(false)
    , m_isAngleCompleted(false)
    , m_isShiftPressed(false)
    , m_infoPanel(nullptr)
    , m_angleLabel(nullptr)
    , m_vertexLabel(nullptr)
    , m_line1Label(nullptr)
    , m_line2Label(nullptr)
    , m_isDarkTheme(false)
{
    createInfoPanel();
}

void AngleMeasurementTool::toggleAngleMode(bool enabled)
{
    m_isAngleMode = enabled;
    
    if (!m_isAngleMode) {
        clearMeasurement();
        m_view->setCursor(Qt::ArrowCursor);
        m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    } else {
        m_view->setCursor(Qt::CrossCursor);
        m_view->setDragMode(QGraphicsView::NoDrag);
    }
    
    emit modeChanged(enabled);
}

void AngleMeasurementTool::handleMousePress(QPointF scenePos)
{
    if (!m_isAngleMode) {
        return;
    }
    
    m_clickCount++;
    
    if (m_clickCount == 1) {
        m_vertex = scenePos;
        m_firstEndPoint = scenePos;
        m_secondEndPoint = scenePos;
        m_isAngleCompleted = false;
        
        m_firstLine = new QGraphicsLineItem();
        m_firstLine->setPen(QPen(Qt::blue, 2, Qt::DashLine));
        m_scene->addItem(m_firstLine);
        
        m_secondLine = new QGraphicsLineItem();
        m_secondLine->setPen(QPen(Qt::green, 2, Qt::DashLine));
        m_scene->addItem(m_secondLine);
        
        m_angleText = new QGraphicsTextItem();
        QFont font;
        font.setPointSize(10);
        font.setBold(true);
        m_angleText->setFont(font);
        m_angleText->setDefaultTextColor(Qt::red);
        m_scene->addItem(m_angleText);
        
        m_angleArc = new QGraphicsPathItem();
        m_angleArc->setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        m_scene->addItem(m_angleArc);
        
        drawAngleMeasurement();
    } else if (m_clickCount == 2) {
        QPointF endPos = scenePos;
        
        if (m_isShiftPressed) {
            endPos = constrainAngle(m_vertex, scenePos, 0);
        }
        
        m_firstEndPoint = endPos;
        drawAngleMeasurement();
    } else if (m_clickCount == 3) {
        QPointF endPos = scenePos;
        
        if (m_isShiftPressed) {
            endPos = constrainAngle(m_vertex, scenePos, 45);
        }
        
        m_secondEndPoint = endPos;
        m_isAngleCompleted = true;
        drawAngleMeasurement();
    } else {
        clearMeasurement();
        m_clickCount = 0;
    }
}

void AngleMeasurementTool::handleMouseMove(QPointF scenePos)
{
    if (!m_isAngleMode) {
        return;
    }
    
    if (m_clickCount == 1) {
        QPointF endPos = scenePos;
        
        if (m_isShiftPressed) {
            endPos = constrainAngle(m_vertex, scenePos, 0);
        }
        
        m_firstEndPoint = endPos;
        m_secondEndPoint = endPos;
        drawAngleMeasurement();
    } else if (m_clickCount == 2) {
        QPointF endPos = scenePos;
        
        if (m_isShiftPressed) {
            endPos = constrainAngle(m_vertex, scenePos, 45);
        }
        
        m_secondEndPoint = endPos;
        drawAngleMeasurement();
    }
}

void AngleMeasurementTool::setShiftPressed(bool pressed)
{
    m_isShiftPressed = pressed;
}

void AngleMeasurementTool::clearMeasurement()
{
    if (m_firstLine != nullptr) {
        m_scene->removeItem(m_firstLine);
        delete m_firstLine;
        m_firstLine = nullptr;
    }
    
    if (m_secondLine != nullptr) {
        m_scene->removeItem(m_secondLine);
        delete m_secondLine;
        m_secondLine = nullptr;
    }
    
    if (m_angleText != nullptr) {
        m_scene->removeItem(m_angleText);
        delete m_angleText;
        m_angleText = nullptr;
    }
    
    if (m_angleArc != nullptr) {
        m_scene->removeItem(m_angleArc);
        delete m_angleArc;
        m_angleArc = nullptr;
    }
    
    m_vertex = QPointF();
    m_firstEndPoint = QPointF();
    m_secondEndPoint = QPointF();
    m_clickCount = 0;
    m_isAngleCompleted = false;
    
    // 重置信息面板
    updateInfoPanel(0, QPointF());
}

void AngleMeasurementTool::updateMeasurementScale()
{
    if (m_firstLine == nullptr || m_secondLine == nullptr || m_angleText == nullptr || m_angleArc == nullptr) {
        return;
    }
    
    drawAngleMeasurement();
}

bool AngleMeasurementTool::isAngleMode() const
{
    return m_isAngleMode;
}

void AngleMeasurementTool::drawAngleMeasurement()
{
    if (m_firstLine == nullptr || m_secondLine == nullptr || m_angleText == nullptr || m_angleArc == nullptr) {
        return;
    }
    
    qreal scaleFactor = m_view->transform().m11();
    
    if (scaleFactor <= 0) {
        scaleFactor = 1.0;
    }
    
    double lineWidth = 2.0 / scaleFactor;
    double fontSize = 10.0 / scaleFactor;
    double textOffset = 25.0 / scaleFactor;
    double arcRadius = 40.0 / scaleFactor;
    
    lineWidth = qMax(lineWidth, 1.0);
    fontSize = qMax(fontSize, 6.0);
    textOffset = qMax(textOffset, 15.0);
    arcRadius = qMax(arcRadius, 20.0);
    
    m_firstLine->setPen(QPen(Qt::blue, lineWidth, Qt::DashLine));
    m_secondLine->setPen(QPen(Qt::green, lineWidth, Qt::DashLine));
    m_angleArc->setPen(QPen(Qt::red, lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    
    m_firstLine->setLine(m_vertex.x(), m_vertex.y(), m_firstEndPoint.x(), m_firstEndPoint.y());
    m_secondLine->setLine(m_vertex.x(), m_vertex.y(), m_secondEndPoint.x(), m_secondEndPoint.y());
    
    double angle1 = calculateAngle(m_vertex, m_firstEndPoint);
    double angle2 = calculateAngle(m_vertex, m_secondEndPoint);
    double angleDiff = calculateAngleDifference(angle1, angle2);
    
    QPainterPath arcPath;
    arcPath.moveTo(m_vertex);
    
    double midAngle = 0.0;
    double textRadius = arcRadius * 1.5;
    
    if (m_clickCount >= 2) {
        double startAngle = qMin(angle1, angle2);
        double endAngle = qMax(angle1, angle2);
        
        if (endAngle - startAngle > 180.0) {
            startAngle = qMax(angle1, angle2);
            endAngle = qMin(angle1, angle2) + 360.0;
        }
        
        midAngle = (startAngle + endAngle) / 2.0;
        if (midAngle >= 360.0) {
            midAngle -= 360.0;
        }
        
        double startAngleRad = startAngle * M_PI / 180.0;
        double endAngleRad = endAngle * M_PI / 180.0;
        
        arcPath.arcTo(m_vertex.x() - arcRadius, m_vertex.y() - arcRadius, 
                     arcRadius * 2, arcRadius * 2, 
                     -startAngle, -(endAngle - startAngle));
        arcPath.lineTo(m_vertex);
    }
    
    m_angleArc->setPath(arcPath);
    
    QString text;
    if (m_clickCount >= 2) {
        text = QString("角度: %1°").arg(angleDiff, 0, 'f', 2);
    } else {
        text = "";
    }
    
    m_angleText->setPlainText(text);
    
    QFont font;
    font.setPointSize(fontSize);
    font.setBold(true);
    m_angleText->setFont(font);
    
    QPointF textPos;
    if (m_clickCount >= 2) {
        double midAngleRad = midAngle * M_PI / 180.0;
        textPos = QPointF(
            m_vertex.x() + textRadius * cos(midAngleRad),
            m_vertex.y() + textRadius * sin(midAngleRad)
        );
    } else {
        textPos = QPointF(
            m_vertex.x() + textOffset,
            m_vertex.y() - textOffset
        );
    }
    m_angleText->setPos(textPos);
    
    // 更新信息面板
    if (m_clickCount >= 2) {
        updateInfoPanel(angleDiff, m_vertex);
    } else {
        updateInfoPanel(0, m_vertex);
    }
}

double AngleMeasurementTool::calculateAngle(const QPointF &p1, const QPointF &p2)
{
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    
    double angle = atan2(dy, dx) * 180.0 / M_PI;
    
    if (angle < 0) {
        angle += 360.0;
    }
    
    return angle;
}

double AngleMeasurementTool::calculateAngleDifference(double angle1, double angle2)
{
    double diff = qAbs(angle1 - angle2);
    
    if (diff > 180.0) {
        diff = 360.0 - diff;
    }
    
    return diff;
}

QPointF AngleMeasurementTool::constrainAngle(const QPointF &vertex, const QPointF &point, double targetAngle)
{
    double dx = point.x() - vertex.x();
    double dy = point.y() - vertex.y();
    
    double distance = sqrt(dx * dx + dy * dy);
    
    if (distance < 1.0) {
        return point;
    }
    
    double currentAngle = atan2(dy, dx) * 180.0 / M_PI;
    
    double constrainedAngle = round(currentAngle / 15.0) * 15.0;
    
    double angleRad = constrainedAngle * M_PI / 180.0;
    
    QPointF constrainedPoint(
        vertex.x() + distance * cos(angleRad),
        vertex.y() + distance * sin(angleRad)
    );
    
    return constrainedPoint;
}

QWidget* AngleMeasurementTool::getInfoPanel() const
{
    return m_infoPanel;
}

void AngleMeasurementTool::updateTheme(bool isDarkTheme)
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

void AngleMeasurementTool::createInfoPanel()
{
    m_infoPanel = new QWidget();
    m_infoPanel->setObjectName("toolPanel");
    QVBoxLayout *layout = new QVBoxLayout(m_infoPanel);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(6);
    
    PanelStyle& style = PanelStyle::instance();
    
    QLabel *titleLabel = style.createTitleLabel("测角工具", m_isDarkTheme);
    layout->addWidget(titleLabel);
    
    QFrame *line0 = style.createSeparator(m_isDarkTheme);
    layout->addWidget(line0);
    
    QLabel *angleTitle = style.createSectionLabel("角度", m_isDarkTheme);
    layout->addWidget(angleTitle);
    
    m_angleLabel = style.createEmphasisLabel("0.00°", m_isDarkTheme);
    layout->addWidget(m_angleLabel);
    
    QFrame *line1 = style.createSeparator(m_isDarkTheme);
    layout->addWidget(line1);
    
    QLabel *vertexTitle = style.createSectionLabel("顶点", m_isDarkTheme);
    layout->addWidget(vertexTitle);
    
    m_vertexLabel = style.createContentLabel("(0.0, 0.0)", m_isDarkTheme);
    layout->addWidget(m_vertexLabel);
    
    QFrame *line2 = style.createSeparator(m_isDarkTheme);
    layout->addWidget(line2);
    
    QLabel *line1Title = style.createSectionLabel("线1角度", m_isDarkTheme);
    layout->addWidget(line1Title);
    
    m_line1Label = style.createContentLabel("0.0°", m_isDarkTheme);
    layout->addWidget(m_line1Label);
    
    QFrame *line3 = style.createSeparator(m_isDarkTheme);
    layout->addWidget(line3);
    
    QLabel *line2Title = style.createSectionLabel("线2角度", m_isDarkTheme);
    layout->addWidget(line2Title);
    
    m_line2Label = style.createContentLabel("0.0°", m_isDarkTheme);
    layout->addWidget(m_line2Label);
    
    layout->addStretch();
    
    style.applyPanelStyle(m_infoPanel, m_isDarkTheme);
}

void AngleMeasurementTool::updateInfoPanel(double angle, const QPointF &vertex)
{
    if (m_angleLabel) {
        m_angleLabel->setText(QString("%1°").arg(angle, 0, 'f', 2));
    }
    if (m_vertexLabel) {
        m_vertexLabel->setText(QString("(%1, %2)").arg(vertex.x(), 0, 'f', 1).arg(vertex.y(), 0, 'f', 1));
    }
    if (m_line1Label && m_clickCount >= 1) {
        double angle1 = calculateAngle(m_vertex, m_firstEndPoint);
        m_line1Label->setText(QString("%1°").arg(angle1, 0, 'f', 1));
    } else if (m_line1Label) {
        m_line1Label->setText(QString("%1°").arg(0.0, 0, 'f', 1));
    }
    if (m_line2Label && m_clickCount >= 2) {
        double angle2 = calculateAngle(m_vertex, m_secondEndPoint);
        m_line2Label->setText(QString("%1°").arg(angle2, 0, 'f', 1));
    } else if (m_line2Label) {
        m_line2Label->setText(QString("%1°").arg(0.0, 0, 'f', 1));
    }
}
