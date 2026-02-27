#include "colorpickertool.h"

#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QPainter>
#include <QFrame>
#include <cmath>
#include <opencv2/opencv.hpp>

ColorPickerTool::ColorPickerTool(QGraphicsScene *scene, QGraphicsView *view, QObject *parent)
    : QObject(parent)
    , m_scene(scene)
    , m_view(view)
    , m_isColorPickerMode(false)
    , m_colorInfoPanel(nullptr)
    , m_colorPreview(nullptr)
    , m_rSpinBox(nullptr)
    , m_gSpinBox(nullptr)
    , m_bSpinBox(nullptr)
    , m_hSpinBox(nullptr)
    , m_sSpinBox(nullptr)
    , m_vSpinBox(nullptr)
    , m_hCvSpinBox(nullptr)
    , m_sCvSpinBox(nullptr)
    , m_vCvSpinBox(nullptr)
    , m_lSpinBox(nullptr)
    , m_aSpinBox(nullptr)
    , m_bLabSpinBox(nullptr)
    , m_hexEdit(nullptr)
    , m_coordLabel(nullptr)
    , m_updating(false)
    , m_isDarkTheme(true)
{
    createColorInfoPanel();
}

ColorPickerTool::~ColorPickerTool()
{
    if (m_colorInfoPanel) {
        delete m_colorInfoPanel;
    }
}

void ColorPickerTool::toggleColorPickerMode(bool enabled)
{
    m_isColorPickerMode = enabled;

    if (!m_isColorPickerMode) {
        // 退出取色模式
        m_view->setCursor(Qt::ArrowCursor);
        m_view->setDragMode(QGraphicsView::ScrollHandDrag);
        m_colorInfoPanel->hide();
    } else {
        // 进入取色模式
        m_view->setCursor(Qt::CrossCursor);
        m_view->setDragMode(QGraphicsView::NoDrag);
        m_colorInfoPanel->show();
    }

    emit modeChanged(enabled);
}

void ColorPickerTool::handleMouseMove(QPointF scenePos)
{
    if (!m_isColorPickerMode || m_image.isNull()) {
        return;
    }

    // 获取场景坐标对应的图像坐标
    int x = static_cast<int>(scenePos.x());
    int y = static_cast<int>(scenePos.y());

    // 检查坐标是否在图像范围内
    if (x < 0 || x >= m_image.width() || y < 0 || y >= m_image.height()) {
        return;
    }

    // 获取像素颜色
    QColor color = m_image.pixelColor(x, y);
    m_currentColor = color;

    // 更新显示
    updateColorDisplay(color);

    // 更新坐标显示
    m_coordLabel->setText(QString("坐标: (%1, %2)").arg(x).arg(y));

    emit colorChanged(color);
}

bool ColorPickerTool::isColorPickerMode() const
{
    return m_isColorPickerMode;
}

void ColorPickerTool::setImage(const QImage &image)
{
    m_image = image;
}

QWidget* ColorPickerTool::getColorInfoPanel() const
{
    return m_colorInfoPanel;
}

void ColorPickerTool::createColorInfoPanel()
{
    // 创建颜色信息面板
    m_colorInfoPanel = new QWidget();
    m_colorInfoPanel->setMinimumWidth(180);
    m_colorInfoPanel->setStyleSheet(
        "QWidget { background-color: #2d2d2d; border-radius: 8px; }"
        "QLabel { color: #ffffff; }"
        "QSpinBox, QDoubleSpinBox { background-color: #3d3d3d; color: #ffffff; border: 1px solid #555555; border-radius: 3px; padding: 2px; }"
        "QSpinBox:focus, QDoubleSpinBox:focus { border: 1px solid #0078d4; }"
        "QLineEdit { background-color: #3d3d3d; color: #ffffff; border: 1px solid #555555; border-radius: 3px; padding: 2px; }"
        "QLineEdit:focus { border: 1px solid #0078d4; }"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(m_colorInfoPanel);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(6);

    // 标题
    QLabel *titleLabel = new QLabel("颜色信息");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 13px; color: #ffffff;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 分隔线
    QFrame *line0 = new QFrame();
    line0->setFrameShape(QFrame::HLine);
    line0->setStyleSheet("background-color: #555555;");
    mainLayout->addWidget(line0);

    // 颜色预览
    QHBoxLayout *previewLayout = new QHBoxLayout();
    m_colorPreview = new QLabel();
    m_colorPreview->setFixedSize(50, 50);
    m_colorPreview->setStyleSheet("background-color: #000000; border: 2px solid #ffffff; border-radius: 5px;");
    m_colorPreview->setScaledContents(true);
    previewLayout->addWidget(m_colorPreview);

    // HEX值输入
    QVBoxLayout *hexLayout = new QVBoxLayout();
    QLabel *hexTitleLabel = new QLabel("HEX:");
    hexTitleLabel->setStyleSheet("font-weight: bold; font-size: 11px;");
    m_hexEdit = new QLineEdit("#000000");
    m_hexEdit->setMaxLength(7);
    m_hexEdit->setFixedWidth(80);
    m_hexEdit->setStyleSheet("font-family: Consolas, monospace;");
    hexLayout->addWidget(hexTitleLabel);
    hexLayout->addWidget(m_hexEdit);
    previewLayout->addLayout(hexLayout);
    previewLayout->addStretch();
    mainLayout->addLayout(previewLayout);

    // 分隔线
    QFrame *line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    line1->setStyleSheet("background-color: #555555;");
    mainLayout->addWidget(line1);

    // RGB 控件
    QLabel *rgbTitle = new QLabel("RGB");
    rgbTitle->setStyleSheet("font-weight: bold; font-size: 11px;");
    mainLayout->addWidget(rgbTitle);

    QHBoxLayout *rgbLayout = new QHBoxLayout();
    QLabel *rLabel = new QLabel("R:");
    m_rSpinBox = new QSpinBox();
    m_rSpinBox->setRange(0, 255);
    m_rSpinBox->setFixedWidth(55);
    QLabel *gLabel = new QLabel("G:");
    m_gSpinBox = new QSpinBox();
    m_gSpinBox->setRange(0, 255);
    m_gSpinBox->setFixedWidth(55);
    QLabel *bLabel = new QLabel("B:");
    m_bSpinBox = new QSpinBox();
    m_bSpinBox->setRange(0, 255);
    m_bSpinBox->setFixedWidth(55);
    rgbLayout->addWidget(rLabel);
    rgbLayout->addWidget(m_rSpinBox);
    rgbLayout->addWidget(gLabel);
    rgbLayout->addWidget(m_gSpinBox);
    rgbLayout->addWidget(bLabel);
    rgbLayout->addWidget(m_bSpinBox);
    rgbLayout->addStretch();
    mainLayout->addLayout(rgbLayout);

    // 分隔线
    QFrame *line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setStyleSheet("background-color: #555555;");
    mainLayout->addWidget(line2);

    // HSV 控件 (标准显示格式)
    QLabel *hsvTitle = new QLabel("HSV");
    hsvTitle->setStyleSheet("font-weight: bold; font-size: 11px;");
    mainLayout->addWidget(hsvTitle);

    QHBoxLayout *hsvLayout = new QHBoxLayout();
    QLabel *hLabel = new QLabel("H:");
    m_hSpinBox = new QSpinBox();
    m_hSpinBox->setRange(0, 360);
    m_hSpinBox->setSuffix("°");
    m_hSpinBox->setFixedWidth(60);
    QLabel *sLabel = new QLabel("S:");
    m_sSpinBox = new QSpinBox();
    m_sSpinBox->setRange(0, 100);
    m_sSpinBox->setSuffix("%");
    m_sSpinBox->setFixedWidth(60);
    QLabel *vLabel = new QLabel("V:");
    m_vSpinBox = new QSpinBox();
    m_vSpinBox->setRange(0, 100);
    m_vSpinBox->setSuffix("%");
    m_vSpinBox->setFixedWidth(60);
    hsvLayout->addWidget(hLabel);
    hsvLayout->addWidget(m_hSpinBox);
    hsvLayout->addWidget(sLabel);
    hsvLayout->addWidget(m_sSpinBox);
    hsvLayout->addWidget(vLabel);
    hsvLayout->addWidget(m_vSpinBox);
    hsvLayout->addStretch();
    mainLayout->addLayout(hsvLayout);

    // 分隔线
    QFrame *line3 = new QFrame();
    line3->setFrameShape(QFrame::HLine);
    line3->setStyleSheet("background-color: #555555;");
    mainLayout->addWidget(line3);

    // HSV(CV) 控件 (OpenCV内部值)
    QLabel *hsvCvTitle = new QLabel("HSV (OpenCV)");
    hsvCvTitle->setStyleSheet("font-weight: bold; font-size: 11px;");
    mainLayout->addWidget(hsvCvTitle);

    QHBoxLayout *hsvCvLayout = new QHBoxLayout();
    QLabel *hCvLabel = new QLabel("H:");
    m_hCvSpinBox = new QSpinBox();
    m_hCvSpinBox->setRange(0, 180);
    m_hCvSpinBox->setFixedWidth(55);
    QLabel *sCvLabel = new QLabel("S:");
    m_sCvSpinBox = new QSpinBox();
    m_sCvSpinBox->setRange(0, 255);
    m_sCvSpinBox->setFixedWidth(55);
    QLabel *vCvLabel = new QLabel("V:");
    m_vCvSpinBox = new QSpinBox();
    m_vCvSpinBox->setRange(0, 255);
    m_vCvSpinBox->setFixedWidth(55);
    hsvCvLayout->addWidget(hCvLabel);
    hsvCvLayout->addWidget(m_hCvSpinBox);
    hsvCvLayout->addWidget(sCvLabel);
    hsvCvLayout->addWidget(m_sCvSpinBox);
    hsvCvLayout->addWidget(vCvLabel);
    hsvCvLayout->addWidget(m_vCvSpinBox);
    hsvCvLayout->addStretch();
    mainLayout->addLayout(hsvCvLayout);

    // 分隔线
    QFrame *line4 = new QFrame();
    line4->setFrameShape(QFrame::HLine);
    line4->setStyleSheet("background-color: #555555;");
    mainLayout->addWidget(line4);

    // Lab 控件
    QLabel *labTitle = new QLabel("Lab");
    labTitle->setStyleSheet("font-weight: bold; font-size: 11px;");
    mainLayout->addWidget(labTitle);

    QHBoxLayout *labLayout = new QHBoxLayout();
    QLabel *lLabel = new QLabel("L:");
    m_lSpinBox = new QDoubleSpinBox();
    m_lSpinBox->setRange(0.0, 100.0);
    m_lSpinBox->setDecimals(2);
    m_lSpinBox->setFixedWidth(60);
    QLabel *aLabel = new QLabel("a:");
    m_aSpinBox = new QDoubleSpinBox();
    m_aSpinBox->setRange(-128.0, 128.0);
    m_aSpinBox->setDecimals(2);
    m_aSpinBox->setFixedWidth(60);
    QLabel *bLabLabel = new QLabel("b:");
    m_bLabSpinBox = new QDoubleSpinBox();
    m_bLabSpinBox->setRange(-128.0, 128.0);
    m_bLabSpinBox->setDecimals(2);
    m_bLabSpinBox->setFixedWidth(60);
    labLayout->addWidget(lLabel);
    labLayout->addWidget(m_lSpinBox);
    labLayout->addWidget(aLabel);
    labLayout->addWidget(m_aSpinBox);
    labLayout->addWidget(bLabLabel);
    labLayout->addWidget(m_bLabSpinBox);
    labLayout->addStretch();
    mainLayout->addLayout(labLayout);

    // 分隔线
    QFrame *line5 = new QFrame();
    line5->setFrameShape(QFrame::HLine);
    line5->setStyleSheet("background-color: #555555;");
    mainLayout->addWidget(line5);

    // 坐标信息
    m_coordLabel = new QLabel("坐标: (0, 0)");
    m_coordLabel->setStyleSheet("font-size: 11px; color: #aaaaaa;");
    mainLayout->addWidget(m_coordLabel);

    mainLayout->addStretch();

    // 连接信号
    connect(m_rSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPickerTool::onRgbChanged);
    connect(m_gSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPickerTool::onRgbChanged);
    connect(m_bSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPickerTool::onRgbChanged);

    connect(m_hSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPickerTool::onHsvChanged);
    connect(m_sSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPickerTool::onHsvChanged);
    connect(m_vSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPickerTool::onHsvChanged);

    connect(m_hCvSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPickerTool::onHsvCvChanged);
    connect(m_sCvSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPickerTool::onHsvCvChanged);
    connect(m_vCvSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPickerTool::onHsvCvChanged);

    connect(m_lSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorPickerTool::onLabChanged);
    connect(m_aSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorPickerTool::onLabChanged);
    connect(m_bLabSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorPickerTool::onLabChanged);

    connect(m_hexEdit, &QLineEdit::editingFinished, this, [this]() {
        onHexChanged(m_hexEdit->text());
    });

    m_colorInfoPanel->hide();
}

void ColorPickerTool::updateColorDisplay(const QColor &color)
{
    m_updating = true;
    blockAllSignals(true);

    // 更新颜色预览
    QString colorStyle = QString("background-color: %1; border: 2px solid #ffffff; border-radius: 5px;")
        .arg(color.name());
    m_colorPreview->setStyleSheet(colorStyle);

    // 更新 HEX
    m_hexEdit->setText(color.name().toUpper());

    // 更新 RGB
    m_rSpinBox->setValue(color.red());
    m_gSpinBox->setValue(color.green());
    m_bSpinBox->setValue(color.blue());

    // 计算并更新 HSV
    int h, s, v, h_cv, s_cv, v_cv;
    rgbToHsv(color.red(), color.green(), color.blue(), h, s, v, h_cv, s_cv, v_cv);
    m_hSpinBox->setValue(h);
    m_sSpinBox->setValue(s);
    m_vSpinBox->setValue(v);

    // 更新 HSV(CV)
    m_hCvSpinBox->setValue(h_cv);
    m_sCvSpinBox->setValue(s_cv);
    m_vCvSpinBox->setValue(v_cv);

    // 计算并更新 Lab
    double L, a, b_val;
    rgbToLab(color.red(), color.green(), color.blue(), L, a, b_val);
    m_lSpinBox->setValue(L);
    m_aSpinBox->setValue(a);
    m_bLabSpinBox->setValue(b_val);

    blockAllSignals(false);
    m_updating = false;
}

void ColorPickerTool::updateAllDisplays()
{
    updateColorDisplay(m_currentColor);
}

void ColorPickerTool::blockAllSignals(bool block)
{
    m_rSpinBox->blockSignals(block);
    m_gSpinBox->blockSignals(block);
    m_bSpinBox->blockSignals(block);
    m_hSpinBox->blockSignals(block);
    m_sSpinBox->blockSignals(block);
    m_vSpinBox->blockSignals(block);
    m_hCvSpinBox->blockSignals(block);
    m_sCvSpinBox->blockSignals(block);
    m_vCvSpinBox->blockSignals(block);
    m_lSpinBox->blockSignals(block);
    m_aSpinBox->blockSignals(block);
    m_bLabSpinBox->blockSignals(block);
    m_hexEdit->blockSignals(block);
}

void ColorPickerTool::onRgbChanged(int)
{
    if (m_updating) return;

    int r = m_rSpinBox->value();
    int g = m_gSpinBox->value();
    int b = m_bSpinBox->value();

    m_currentColor = QColor(r, g, b);
    updateAllDisplays();
    emit colorChanged(m_currentColor);
}

void ColorPickerTool::onHsvChanged(int)
{
    if (m_updating) return;

    int h = m_hSpinBox->value();
    int s = m_sSpinBox->value();
    int v = m_vSpinBox->value();

    int r, g, b;
    hsvToRgb(h, s, v, r, g, b);

    m_currentColor = QColor(r, g, b);
    updateAllDisplays();
    emit colorChanged(m_currentColor);
}

void ColorPickerTool::onHsvCvChanged(int)
{
    if (m_updating) return;

    int h_cv = m_hCvSpinBox->value();
    int s_cv = m_sCvSpinBox->value();
    int v_cv = m_vCvSpinBox->value();

    int r, g, b;
    hsvCvToRgb(h_cv, s_cv, v_cv, r, g, b);

    m_currentColor = QColor(r, g, b);
    updateAllDisplays();
    emit colorChanged(m_currentColor);
}

void ColorPickerTool::onLabChanged(double)
{
    if (m_updating) return;

    double L = m_lSpinBox->value();
    double a = m_aSpinBox->value();
    double b_val = m_bLabSpinBox->value();

    int r, g, b;
    labToRgb(L, a, b_val, r, g, b);

    m_currentColor = QColor(r, g, b);
    updateAllDisplays();
    emit colorChanged(m_currentColor);
}

void ColorPickerTool::onHexChanged(const QString &text)
{
    if (m_updating) return;

    QString hex = text.trimmed();
    if (!hex.startsWith('#')) {
        hex = '#' + hex;
    }

    if (hex.length() == 7) {
        QColor color(hex);
        if (color.isValid()) {
            m_currentColor = color;
            updateAllDisplays();
            emit colorChanged(m_currentColor);
        }
    }
}

void ColorPickerTool::rgbToHsv(int r, int g, int b, int &h, int &s, int &v, int &h_cv, int &s_cv, int &v_cv)
{
    // 使用 OpenCV 进行 RGB 到 HSV 转换
    cv::Mat rgbMat(1, 1, CV_8UC3, cv::Scalar(b, g, r));  // OpenCV 默认 BGR 顺序
    cv::Mat hsvMat;
    cv::cvtColor(rgbMat, hsvMat, cv::COLOR_BGR2HSV);

    // OpenCV HSV 范围: H=[0,180], S=[0,255], V=[0,255]
    cv::Vec3b hsv = hsvMat.at<cv::Vec3b>(0, 0);

    // 保存 OpenCV 内部值
    h_cv = hsv[0];
    s_cv = hsv[1];
    v_cv = hsv[2];

    // 转换为标准显示格式: H=[0,360], S=[0,100%], V=[0,100%]
    h = hsv[0] * 2;         // H: 0-180 -> 0-360
    s = hsv[1] * 100 / 255; // S: 0-255 -> 0-100%
    v = hsv[2] * 100 / 255; // V: 0-255 -> 0-100%
}

void ColorPickerTool::hsvToRgb(int h, int s, int v, int &r, int &g, int &b)
{
    // 将标准HSV转换为OpenCV内部格式
    int h_cv = h / 2;           // H: 0-360 -> 0-180
    int s_cv = s * 255 / 100;   // S: 0-100% -> 0-255
    int v_cv = v * 255 / 100;   // V: 0-100% -> 0-255

    hsvCvToRgb(h_cv, s_cv, v_cv, r, g, b);
}

void ColorPickerTool::hsvCvToRgb(int h_cv, int s_cv, int v_cv, int &r, int &g, int &b)
{
    // 使用 OpenCV 进行 HSV 到 RGB 转换
    cv::Mat hsvMat(1, 1, CV_8UC3, cv::Scalar(h_cv, s_cv, v_cv));
    cv::Mat rgbMat;
    cv::cvtColor(hsvMat, rgbMat, cv::COLOR_HSV2BGR);

    cv::Vec3b rgb = rgbMat.at<cv::Vec3b>(0, 0);
    b = rgb[0];  // OpenCV 是 BGR 顺序
    g = rgb[1];
    r = rgb[2];
}

void ColorPickerTool::rgbToLab(int r, int g, int b, double &L, double &a, double &b_val)
{
    // 先转换到XYZ
    double x, y, z;
    rgbToXyz(r, g, b, x, y, z);

    // D65参考白点
    const double Xn = 0.95047;
    const double Yn = 1.0;
    const double Zn = 1.08883;

    // 归一化XYZ值
    x /= Xn;
    y /= Yn;
    z /= Zn;

    // f函数
    auto f = [](double t) -> double {
        const double delta = 6.0 / 29.0;
        if (t > delta * delta * delta) {
            return pow(t, 1.0 / 3.0);
        } else {
            return t / (3.0 * delta * delta) + 4.0 / 29.0;
        }
    };

    // 计算Lab值
    L = 116.0 * f(y) - 16.0;
    a = 500.0 * (f(x) - f(y));
    b_val = 200.0 * (f(y) - f(z));
}

void ColorPickerTool::labToRgb(double L, double a, double b_val, int &r, int &g, int &bl)
{
    // D65参考白点
    const double Xn = 0.95047;
    const double Yn = 1.0;
    const double Zn = 1.08883;

    // f逆函数
    auto fInv = [](double t) -> double {
        const double delta = 6.0 / 29.0;
        if (t > delta) {
            return t * t * t;
        } else {
            return 3.0 * delta * delta * (t - 4.0 / 29.0);
        }
    };

    // 计算XYZ值
    double fy = (L + 16.0) / 116.0;
    double fx = a / 500.0 + fy;
    double fz = fy - b_val / 200.0;

    double x = Xn * fInv(fx);
    double y = Yn * fInv(fy);
    double z = Zn * fInv(fz);

    xyzToRgb(x, y, z, r, g, bl);
}

void ColorPickerTool::rgbToXyz(int r, int g, int b, double &x, double &y, double &z)
{
    // 将RGB值从[0,255]转换到[0,1]
    double rNorm = r / 255.0;
    double gNorm = g / 255.0;
    double bNorm = b / 255.0;

    // 进行伽马逆变换（sRGB到线性RGB）
    auto inverseGamma = [](double c) -> double {
        if (c > 0.04045) {
            return pow((c + 0.055) / 1.055, 2.4);
        } else {
            return c / 12.92;
        }
    };

    rNorm = inverseGamma(rNorm);
    gNorm = inverseGamma(gNorm);
    bNorm = inverseGamma(bNorm);

    // 转换到XYZ色彩空间（使用D65参考白点）
    x = rNorm * 0.4124564 + gNorm * 0.3575761 + bNorm * 0.1804375;
    y = rNorm * 0.2126729 + gNorm * 0.7151522 + bNorm * 0.0721750;
    z = rNorm * 0.0193339 + gNorm * 0.1191920 + bNorm * 0.9503041;
}

void ColorPickerTool::xyzToRgb(double x, double y, double z, int &r, int &g, int &b)
{
    // XYZ到线性RGB的转换矩阵
    double rNorm = x * 3.2404542 + y * -1.5371385 + z * -0.4985314;
    double gNorm = x * -0.9692660 + y * 1.8760108 + z * 0.0415560;
    double bNorm = x * 0.0556434 + y * -0.2040259 + z * 1.0572252;

    // 进行伽马变换（线性RGB到sRGB）
    auto gamma = [](double c) -> double {
        if (c > 0.0031308) {
            return 1.055 * pow(c, 1.0 / 2.4) - 0.055;
        } else {
            return 12.92 * c;
        }
    };

    rNorm = gamma(rNorm);
    gNorm = gamma(gNorm);
    bNorm = gamma(bNorm);

    // 将RGB值从[0,1]转换到[0,255]并钳制
    r = qBound(0, static_cast<int>(rNorm * 255.0 + 0.5), 255);
    g = qBound(0, static_cast<int>(gNorm * 255.0 + 0.5), 255);
    b = qBound(0, static_cast<int>(bNorm * 255.0 + 0.5), 255);
}

void ColorPickerTool::updateTheme(bool isDarkTheme)
{
    m_isDarkTheme = isDarkTheme;

    if (isDarkTheme) {
        // 深色主题
        m_colorInfoPanel->setStyleSheet(
            "QWidget { background-color: #2d2d2d; border-radius: 8px; }"
            "QLabel { color: #ffffff; }"
            "QSpinBox, QDoubleSpinBox { background-color: #3d3d3d; color: #ffffff; border: 1px solid #555555; border-radius: 3px; padding: 2px; }"
            "QSpinBox:focus, QDoubleSpinBox:focus { border: 1px solid #0078d4; }"
            "QLineEdit { background-color: #3d3d3d; color: #ffffff; border: 1px solid #555555; border-radius: 3px; padding: 2px; }"
            "QLineEdit:focus { border: 1px solid #0078d4; }"
        );
        m_colorPreview->setStyleSheet(QString("background-color: %1; border: 2px solid #ffffff; border-radius: 5px;").arg(m_currentColor.name()));
    } else {
        // 浅色主题
        m_colorInfoPanel->setStyleSheet(
            "QWidget { background-color: #f5f5f5; border-radius: 8px; }"
            "QLabel { color: #000000; }"
            "QSpinBox, QDoubleSpinBox { background-color: #ffffff; color: #000000; border: 1px solid #cccccc; border-radius: 3px; padding: 2px; }"
            "QSpinBox:focus, QDoubleSpinBox:focus { border: 1px solid #0078d4; }"
            "QLineEdit { background-color: #ffffff; color: #000000; border: 1px solid #cccccc; border-radius: 3px; padding: 2px; }"
            "QLineEdit:focus { border: 1px solid #0078d4; }"
        );
        m_colorPreview->setStyleSheet(QString("background-color: %1; border: 2px solid #333333; border-radius: 5px;").arg(m_currentColor.name()));
    }
}