#ifndef PANELSTYLE_H
#define PANELSTYLE_H

#include <QString>
#include <QFont>
#include <QFrame>
#include <QLabel>

class PanelStyle
{
public:
    static PanelStyle& instance();

    QString getPanelStyleSheet(bool isDarkTheme) const;
    QString getSeparatorStyleSheet(bool isDarkTheme) const;
    QString getInputStyleSheet(bool isDarkTheme) const;
    QString getSpinBoxStyleSheet(bool isDarkTheme) const;
    QString getSliderStyleSheet(bool isDarkTheme) const;
    QString getButtonStyleSheet(bool isDarkTheme) const;
    QString getGroupBoxStyleSheet(bool isDarkTheme) const;

    QFont getTitleFont() const;
    QFont getSectionFont() const;
    QFont getContentFont() const;
    QFont getEmphasisFont() const;

    QFrame* createSeparator(bool isDarkTheme) const;
    QLabel* createTitleLabel(const QString& text, bool isDarkTheme) const;
    QLabel* createSectionLabel(const QString& text, bool isDarkTheme) const;
    QLabel* createContentLabel(const QString& text, bool isDarkTheme) const;
    QLabel* createEmphasisLabel(const QString& text, bool isDarkTheme) const;

    void applyPanelStyle(QWidget* panel, bool isDarkTheme) const;

private:
    PanelStyle() = default;
    PanelStyle(const PanelStyle&) = delete;
    PanelStyle& operator=(const PanelStyle&) = delete;

    mutable QFont m_titleFont;
    mutable QFont m_sectionFont;
    mutable QFont m_contentFont;
    mutable QFont m_emphasisFont;
    mutable bool m_fontsInitialized = false;

    void ensureFontsInitialized() const;
};

#endif
