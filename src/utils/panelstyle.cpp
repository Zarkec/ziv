#include "panelstyle.h"
#include <QWidget>

PanelStyle& PanelStyle::instance()
{
    static PanelStyle instance;
    return instance;
}

QString PanelStyle::getPanelStyleSheet(bool isDarkTheme) const
{
    if (isDarkTheme) {
        return QString(
            "QWidget#toolPanel { background-color: #2d2d2d; border-radius: 8px; }"
            "QLabel { color: #ffffff; background-color: transparent; }"
        );
    } else {
        return QString(
            "QWidget#toolPanel { background-color: #f5f5f5; border-radius: 8px; }"
            "QLabel { color: #000000; background-color: transparent; }"
        );
    }
}

QString PanelStyle::getSeparatorStyleSheet(bool isDarkTheme) const
{
    return isDarkTheme ? "background-color: #555555;" : "background-color: #cccccc;";
}

QString PanelStyle::getInputStyleSheet(bool isDarkTheme) const
{
    if (isDarkTheme) {
        return QString(
            "QLineEdit { background-color: #3d3d3d; color: #ffffff; border: 1px solid #555555; border-radius: 3px; padding: 2px; }"
            "QLineEdit:focus { border: 1px solid #0078d4; }"
        );
    } else {
        return QString(
            "QLineEdit { background-color: #ffffff; color: #000000; border: 1px solid #cccccc; border-radius: 3px; padding: 2px; }"
            "QLineEdit:focus { border: 1px solid #0078d4; }"
        );
    }
}

QString PanelStyle::getSpinBoxStyleSheet(bool isDarkTheme) const
{
    Q_UNUSED(isDarkTheme);
    return QString();
}

QString PanelStyle::getSliderStyleSheet(bool isDarkTheme) const
{
    Q_UNUSED(isDarkTheme);
    return QString();
}

QString PanelStyle::getButtonStyleSheet(bool isDarkTheme) const
{
    Q_UNUSED(isDarkTheme);
    return QString();
}

QString PanelStyle::getGroupBoxStyleSheet(bool isDarkTheme) const
{
    if (isDarkTheme) {
        return QString(
            "QGroupBox { background-color: transparent; color: #ffffff; border: 1px solid #555555; border-radius: 4px; margin-top: 10px; padding-top: 10px; }"
            "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
        );
    } else {
        return QString(
            "QGroupBox { background-color: transparent; color: #000000; border: 1px solid #cccccc; border-radius: 4px; margin-top: 10px; padding-top: 10px; }"
            "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
        );
    }
}

QFont PanelStyle::getTitleFont() const
{
    ensureFontsInitialized();
    return m_titleFont;
}

QFont PanelStyle::getSectionFont() const
{
    ensureFontsInitialized();
    return m_sectionFont;
}

QFont PanelStyle::getContentFont() const
{
    ensureFontsInitialized();
    return m_contentFont;
}

QFont PanelStyle::getEmphasisFont() const
{
    ensureFontsInitialized();
    return m_emphasisFont;
}

void PanelStyle::ensureFontsInitialized() const
{
    if (!m_fontsInitialized) {
        m_titleFont.setBold(true);
        m_titleFont.setPointSize(12);

        m_sectionFont.setBold(true);
        m_sectionFont.setPointSize(10);

        m_contentFont.setPointSize(10);

        m_emphasisFont.setBold(true);
        m_emphasisFont.setPointSize(14);

        m_fontsInitialized = true;
    }
}

QFrame* PanelStyle::createSeparator(bool isDarkTheme) const
{
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(getSeparatorStyleSheet(isDarkTheme));
    return line;
}

QLabel* PanelStyle::createTitleLabel(const QString& text, bool isDarkTheme) const
{
    QLabel* label = new QLabel(text);
    label->setFont(getTitleFont());
    label->setAlignment(Qt::AlignLeft);
    Q_UNUSED(isDarkTheme);
    return label;
}

QLabel* PanelStyle::createSectionLabel(const QString& text, bool isDarkTheme) const
{
    QLabel* label = new QLabel(text);
    label->setFont(getSectionFont());
    Q_UNUSED(isDarkTheme);
    return label;
}

QLabel* PanelStyle::createContentLabel(const QString& text, bool isDarkTheme) const
{
    QLabel* label = new QLabel(text);
    label->setFont(getContentFont());
    label->setWordWrap(true);
    Q_UNUSED(isDarkTheme);
    return label;
}

QLabel* PanelStyle::createEmphasisLabel(const QString& text, bool isDarkTheme) const
{
    QLabel* label = new QLabel(text);
    label->setFont(getEmphasisFont());
    label->setWordWrap(true);
    Q_UNUSED(isDarkTheme);
    return label;
}

void PanelStyle::applyPanelStyle(QWidget* panel, bool isDarkTheme) const
{
    if (!panel) return;

    panel->setObjectName("toolPanel");
    QString style = getPanelStyleSheet(isDarkTheme);
    style += getInputStyleSheet(isDarkTheme);
    style += getSpinBoxStyleSheet(isDarkTheme);
    style += getSliderStyleSheet(isDarkTheme);
    style += getButtonStyleSheet(isDarkTheme);
    style += getGroupBoxStyleSheet(isDarkTheme);
    panel->setStyleSheet(style);
}
