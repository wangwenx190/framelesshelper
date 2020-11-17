/*
 * MIT License
 *
 * Copyright (C) 2020 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <QWidget>

#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
#define Q_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;

#define Q_DISABLE_COPY_MOVE(Class) \
    Q_DISABLE_COPY(Class) \
    Q_DISABLE_MOVE(Class)
#endif

QT_FORWARD_DECLARE_CLASS(QVBoxLayout)
QT_FORWARD_DECLARE_CLASS(QHBoxLayout)
QT_FORWARD_DECLARE_CLASS(QSpacerItem)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QCheckBox)

class Widget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Widget)

public:
    enum class Win10Version : int {
        Win10_1507 = 10240,
        Win10_1511 = 10586,
        Win10_1607 = 14393,
        Win10_1703 = 15063,
        Win10_1709 = 16299,
        Win10_1803 = 17134,
        Win10_1809 = 17763,
        Win10_1903 = 18362,
        Win10_1909 = 18363,
        Win10_2004 = 19041,
        Win10_20H2 = 19042,
        Windows10 = Win10_1507
    };
    Q_ENUM(Win10Version)

    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override = default;

    bool isNormaled() const;

    bool shouldDrawBorder(const bool ignoreWindowState = false) const;
    bool shouldDrawThemedBorder(const bool ignoreWindowState = false) const;
    bool shouldDrawThemedTitleBar() const;

    static QColor activeBorderColor();
    static QColor inactiveBorderColor();
    QColor borderColor() const;

    static bool isWin10OrGreater(const Win10Version subVer = Win10Version::Windows10);

public Q_SLOTS:
    void retranslateUi();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#endif
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUi();
    void updateWindow();
    void updateTitleBar();
    void initializeOptions();
    void setupConnections();
    void initializeFramelessFunctions();
    void initializeVariables();
    void initializeWindow();

private:
    bool m_bIsWin10OrGreater = false, m_bCanAcrylicBeEnabled = false, m_bExtendToTitleBar = false,
         m_bCanShowColorDialog = false;
    QVBoxLayout *verticalLayout_3 = nullptr, *verticalLayout_2 = nullptr, *verticalLayout = nullptr;
    QWidget *titleBarWidget = nullptr, *contentsWidget = nullptr, *controlPanelWidget = nullptr;
    QHBoxLayout *horizontalLayout = nullptr, *horizontalLayout_2 = nullptr,
                *horizontalLayout_3 = nullptr;
    QSpacerItem *horizontalSpacer_7 = nullptr, *horizontalSpacer = nullptr,
                *horizontalSpacer_2 = nullptr, *verticalSpacer_2 = nullptr,
                *horizontalSpacer_3 = nullptr, *horizontalSpacer_4 = nullptr,
                *verticalSpacer = nullptr, *horizontalSpacer_5 = nullptr,
                *horizontalSpacer_6 = nullptr;
    QPushButton *iconButton = nullptr, *minimizeButton = nullptr, *maximizeButton = nullptr,
                *closeButton = nullptr, *moveCenterButton = nullptr;
    QLabel *titleLabel = nullptr;
    QCheckBox *customizeTitleBarCB = nullptr, *preserveWindowFrameCB = nullptr,
              *blurEffectCB = nullptr, *extendToTitleBarCB = nullptr, *forceAcrylicCB = nullptr,
              *resizableCB = nullptr;
};
