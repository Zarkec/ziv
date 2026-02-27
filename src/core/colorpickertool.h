#ifndef COLORPICKERTOOL_H
#define COLORPICKERTOOL_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPointF>
#include <QColor>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>

/**
 * @brief 取色器工具类
 *
 * 实现图片取色功能，支持显示和编辑RGB、HSV和Lab颜色空间参数
 */
class ColorPickerTool : public QObject
{
    Q_OBJECT

public:
    explicit ColorPickerTool(QGraphicsScene *scene, QGraphicsView *view, QObject *parent = nullptr);
    ~ColorPickerTool();

    // 切换取色模式
    void toggleColorPickerMode(bool enabled);

    // 处理鼠标移动事件
    void handleMouseMove(QPointF scenePos);

    // 获取当前是否处于取色模式
    bool isColorPickerMode() const;

    // 设置图像用于颜色采样
    void setImage(const QImage &image);

    // 获取颜色信息面板
    QWidget* getColorInfoPanel() const;

signals:
    // 模式改变信号
    void modeChanged(bool enabled);

    // 颜色改变信号
    void colorChanged(const QColor &color);

private slots:
    // RGB 值改变槽函数
    void onRgbChanged(int value);

    // HSV 值改变槽函数
    void onHsvChanged(int value);

    // HSV(CV) 值改变槽函数
    void onHsvCvChanged(int value);

    // Lab 值改变槽函数
    void onLabChanged(double value);

    // HEX 值改变槽函数
    void onHexChanged(const QString &text);

private:
    // 创建颜色信息面板
    void createColorInfoPanel();

    // 更新颜色显示（不触发信号）
    void updateColorDisplay(const QColor &color);

    // 更新所有显示（从当前颜色）
    void updateAllDisplays();

    // 阻塞信号更新辅助函数
    void blockAllSignals(bool block);

    // RGB转HSV (返回 OpenCV 内部值和显示值)
    void rgbToHsv(int r, int g, int b, int &h, int &s, int &v, int &h_cv, int &s_cv, int &v_cv);

    // HSV转RGB
    void hsvToRgb(int h, int s, int v, int &r, int &g, int &b);

    // HSV(CV)转RGB
    void hsvCvToRgb(int h_cv, int s_cv, int v_cv, int &r, int &g, int &b);

    // RGB转Lab
    void rgbToLab(int r, int g, int b, double &L, double &a, double &b_val);

    // Lab转RGB
    void labToRgb(double L, double a, double b_val, int &r, int &g, int &bl);

    // RGB转XYZ（Lab转换的中间步骤）
    void rgbToXyz(int r, int g, int b, double &x, double &y, double &z);

    // XYZ转RGB
    void xyzToRgb(double x, double y, double z, int &r, int &g, int &b);

private:
    QGraphicsScene *m_scene;
    QGraphicsView *m_view;

    bool m_isColorPickerMode;
    QImage m_image;

    // 颜色信息面板
    QWidget *m_colorInfoPanel;

    // 颜色预览标签
    QLabel *m_colorPreview;

    // RGB 控件
    QSpinBox *m_rSpinBox;
    QSpinBox *m_gSpinBox;
    QSpinBox *m_bSpinBox;

    // HSV 控件 (标准显示格式: H=0-360, S=0-100%, V=0-100%)
    QSpinBox *m_hSpinBox;
    QSpinBox *m_sSpinBox;
    QSpinBox *m_vSpinBox;

    // HSV(CV) 控件 (OpenCV内部值: H=0-180, S=0-255, V=0-255)
    QSpinBox *m_hCvSpinBox;
    QSpinBox *m_sCvSpinBox;
    QSpinBox *m_vCvSpinBox;

    // Lab 控件
    QDoubleSpinBox *m_lSpinBox;
    QDoubleSpinBox *m_aSpinBox;
    QDoubleSpinBox *m_bLabSpinBox;

    // HEX 输入
    QLineEdit *m_hexEdit;

    // 坐标标签
    QLabel *m_coordLabel;

    // 当前颜色
    QColor m_currentColor;

    // 标志：正在程序化更新，避免循环触发
    bool m_updating;
};

#endif // COLORPICKERTOOL_H