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

#include "widget.h"
#include "../../winnativeeventfilter.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QOperatingSystemVersion>
#include <QPainter>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <qt_windows.h>

// Some old SDK doesn't have this value.
#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif

// Copied from windowsx.h
#define GET_X_LPARAM(lp) ((int) (short) LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int) (short) HIWORD(lp))

#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
#define QLatin1String(str) QString::fromUtf8(str)
#endif

namespace {

const Widget::Win10Version g_vAcrylicEffectVersion = Widget::Win10Version::Win10_1803;

const QColor g_cDefaultActiveBorderColor = {"#707070"};
const QColor g_cDefaultInactiveBorderColor = {"#aaaaaa"};
QColor g_cColorizationColor = Qt::white;

const char g_sUseNativeTitleBar[] = "WNEF_USE_NATIVE_TITLE_BAR";
const char g_sPreserveWindowFrame[] = "WNEF_FORCE_PRESERVE_WINDOW_FRAME";
const char g_sForceUseAcrylicEffect[] = "WNEF_FORCE_ACRYLIC_ON_WIN10";
const char g_sDontExtendFrame[] = "WNEF_DO_NOT_EXTEND_FRAME";

const QLatin1String g_sSystemButtonsStyleSheet(R"(
#iconButton, #minimizeButton, #maximizeButton, #closeButton {
  background-color: transparent;
  border-radius: 0px;
}

#minimizeButton:hover, #maximizeButton:hover {
  border-style: none;
  background-color: #80c7c7c7;
}

#minimizeButton:pressed, #maximizeButton:pressed {
  border-style: none;
  background-color: #80808080;
}

#closeButton:hover {
  border-style: none;
  background-color: #e81123;
}

#closeButton:pressed {
  border-style: none;
  background-color: #8c0a15;
}
)");
const QLatin1String g_sTitleLabelStyleSheet(R"(
#titleLabel {
  color: rgb(%1, %2, %3);
}
)");
const QLatin1String g_sTitleBarStyleSheet(R"(
#titleBarWidget {
  background-color: rgba(%1, %2, %3, %4);
  border-top: 1px solid rgba(%5, %6, %7, %8);
}
)");

const QLatin1String g_sMinimizeButtonImageDark(":/images/button_minimize_black.svg");
const QLatin1String g_sMaximizeButtonImageDark(":/images/button_maximize_black.svg");
const QLatin1String g_sRestoreButtonImageDark(":/images/button_restore_black.svg");
const QLatin1String g_sCloseButtonImageDark(":/images/button_close_black.svg");
const QLatin1String g_sMinimizeButtonImageLight(":/images/button_minimize_white.svg");
const QLatin1String g_sMaximizeButtonImageLight(":/images/button_maximize_white.svg");
const QLatin1String g_sRestoreButtonImageLight(":/images/button_restore_white.svg");
const QLatin1String g_sCloseButtonImageLight(":/images/button_close_white.svg");

} // namespace

Widget::Widget(QWidget *parent) : QWidget(parent)
{
    initializeWindow();
}

void Widget::retranslateUi()
{
    setWindowTitle(tr("Widget"));
    iconButton->setText({});
    titleLabel->setText({});
    minimizeButton->setText({});
    maximizeButton->setText({});
    closeButton->setText({});
    customizeTitleBarCB->setText(tr("Enable customized title bar"));
    preserveWindowFrameCB->setText(tr("Preserve window frame"));
    blurEffectCB->setText(tr("Enable blur effect"));
    extendToTitleBarCB->setText(tr("Extend to title bar"));
    forceAcrylicCB->setText(tr("Force enabling Acrylic effect"));
    resizableCB->setText(tr("Resizable"));
    moveCenterButton->setText(tr("Move to desktop center"));
}

void Widget::setupUi()
{
    resize(1056, 600);
    verticalLayout_3 = new QVBoxLayout(this);
    titleBarWidget = new QWidget(this);
    titleBarWidget->setObjectName(QLatin1String("titleBarWidget"));
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(titleBarWidget->sizePolicy().hasHeightForWidth());
    titleBarWidget->setSizePolicy(sizePolicy);
    const int titleBarHeight
        = WinNativeEventFilter::getSystemMetric(rawHandle(),
                                                WinNativeEventFilter::SystemMetric::TitleBarHeight);
    titleBarWidget->setMinimumSize(QSize(0, titleBarHeight));
    titleBarWidget->setMaximumSize(QSize(16777215, titleBarHeight));
    horizontalLayout = new QHBoxLayout(titleBarWidget);
    horizontalLayout->setSpacing(0);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalSpacer_7 = new QSpacerItem(3, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(horizontalSpacer_7);
    iconButton = new QPushButton(titleBarWidget);
    iconButton->setObjectName(QLatin1String("iconButton"));
    horizontalLayout->addWidget(iconButton);
    horizontalSpacer = new QSpacerItem(3, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(horizontalSpacer);
    titleLabel = new QLabel(titleBarWidget);
    titleLabel->setObjectName(QLatin1String("titleLabel"));
    QFont font;
    font.setPointSize(10);
    titleLabel->setFont(font);
    horizontalLayout->addWidget(titleLabel);
    horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalLayout->addItem(horizontalSpacer_2);
    minimizeButton = new QPushButton(titleBarWidget);
    minimizeButton->setObjectName(QLatin1String("minimizeButton"));
    QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(minimizeButton->sizePolicy().hasHeightForWidth());
    minimizeButton->setSizePolicy(sizePolicy1);
    const QSize systemButtonSize = {45, 30};
    minimizeButton->setMinimumSize(systemButtonSize);
    minimizeButton->setMaximumSize(systemButtonSize);
    QIcon icon;
    icon.addFile(g_sMinimizeButtonImageDark, {}, QIcon::Normal, QIcon::Off);
    minimizeButton->setIcon(icon);
    minimizeButton->setIconSize(systemButtonSize);
    horizontalLayout->addWidget(minimizeButton);
    maximizeButton = new QPushButton(titleBarWidget);
    maximizeButton->setObjectName(QLatin1String("maximizeButton"));
    sizePolicy1.setHeightForWidth(maximizeButton->sizePolicy().hasHeightForWidth());
    maximizeButton->setSizePolicy(sizePolicy1);
    maximizeButton->setMinimumSize(systemButtonSize);
    maximizeButton->setMaximumSize(systemButtonSize);
    QIcon icon1;
    icon1.addFile(g_sMaximizeButtonImageDark, {}, QIcon::Normal, QIcon::Off);
    maximizeButton->setIcon(icon1);
    maximizeButton->setIconSize(systemButtonSize);
    horizontalLayout->addWidget(maximizeButton);
    closeButton = new QPushButton(titleBarWidget);
    closeButton->setObjectName(QLatin1String("closeButton"));
    sizePolicy1.setHeightForWidth(closeButton->sizePolicy().hasHeightForWidth());
    closeButton->setSizePolicy(sizePolicy1);
    closeButton->setMinimumSize(systemButtonSize);
    closeButton->setMaximumSize(systemButtonSize);
    QIcon icon2;
    icon2.addFile(g_sCloseButtonImageDark, {}, QIcon::Normal, QIcon::Off);
    closeButton->setIcon(icon2);
    closeButton->setIconSize(systemButtonSize);
    horizontalLayout->addWidget(closeButton);
    verticalLayout_3->addWidget(titleBarWidget);
    contentsWidget = new QWidget(this);
    QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(contentsWidget->sizePolicy().hasHeightForWidth());
    contentsWidget->setSizePolicy(sizePolicy2);
    verticalLayout_2 = new QVBoxLayout(contentsWidget);
    verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalLayout_2->addItem(verticalSpacer_2);
    horizontalLayout_2 = new QHBoxLayout();
    horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalLayout_2->addItem(horizontalSpacer_3);
    controlPanelWidget = new QWidget(contentsWidget);
    QSizePolicy sizePolicy3(QSizePolicy::Maximum, QSizePolicy::Maximum);
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(controlPanelWidget->sizePolicy().hasHeightForWidth());
    controlPanelWidget->setSizePolicy(sizePolicy3);
    verticalLayout = new QVBoxLayout(controlPanelWidget);
    customizeTitleBarCB = new QCheckBox(controlPanelWidget);
    QFont font1;
    font1.setPointSize(15);
    font1.setBold(true);
    font1.setWeight(75);
    customizeTitleBarCB->setFont(font1);
    verticalLayout->addWidget(customizeTitleBarCB);
    preserveWindowFrameCB = new QCheckBox(controlPanelWidget);
    preserveWindowFrameCB->setFont(font1);
    verticalLayout->addWidget(preserveWindowFrameCB);
    blurEffectCB = new QCheckBox(controlPanelWidget);
    blurEffectCB->setFont(font1);
    verticalLayout->addWidget(blurEffectCB);
    extendToTitleBarCB = new QCheckBox(controlPanelWidget);
    extendToTitleBarCB->setFont(font1);
    verticalLayout->addWidget(extendToTitleBarCB);
    forceAcrylicCB = new QCheckBox(controlPanelWidget);
    forceAcrylicCB->setFont(font1);
    forceAcrylicCB->setEnabled(m_bCanAcrylicBeEnabled);
    verticalLayout->addWidget(forceAcrylicCB);
    resizableCB = new QCheckBox(controlPanelWidget);
    resizableCB->setFont(font1);
    verticalLayout->addWidget(resizableCB);
    horizontalLayout_2->addWidget(controlPanelWidget);
    horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalLayout_2->addItem(horizontalSpacer_4);
    verticalLayout_2->addLayout(horizontalLayout_2);
    verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalLayout_2->addItem(verticalSpacer);
    horizontalLayout_3 = new QHBoxLayout();
    horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalLayout_3->addItem(horizontalSpacer_5);
    moveCenterButton = new QPushButton(contentsWidget);
    moveCenterButton->setFont(font1);
    horizontalLayout_3->addWidget(moveCenterButton);
    horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalLayout_3->addItem(horizontalSpacer_6);
    verticalLayout_2->addLayout(horizontalLayout_3);
    verticalLayout_3->addWidget(contentsWidget);
    if (shouldDrawBorder()) {
        layout()->setContentsMargins(1, 1, 1, 1);
    } else {
        layout()->setContentsMargins(0, 0, 0, 0);
    }
    retranslateUi();
}

bool Widget::isNormaled() const
{
    return !isMinimized() && !isMaximized() && !isFullScreen();
}

bool Widget::shouldDrawBorder(const bool ignoreWindowState) const
{
    return m_bIsWin10OrGreater && (ignoreWindowState ? true : isNormaled())
           && !preserveWindowFrameCB->isChecked() && customizeTitleBarCB->isChecked();
}

bool Widget::shouldDrawThemedBorder(const bool ignoreWindowState) const
{
    return (shouldDrawBorder(ignoreWindowState) && WinNativeEventFilter::colorizationEnabled());
}

bool Widget::shouldDrawThemedTitleBar() const
{
    return m_bIsWin10OrGreater && WinNativeEventFilter::colorizationEnabled();
}

QColor Widget::activeBorderColor()
{
    return WinNativeEventFilter::colorizationEnabled() ? g_cColorizationColor
                                                       : g_cDefaultActiveBorderColor;
}

QColor Widget::inactiveBorderColor()
{
    return g_cDefaultInactiveBorderColor;
}

QColor Widget::borderColor() const
{
    return isActiveWindow() ? activeBorderColor() : inactiveBorderColor();
}

bool Widget::isWin10OrGreater(const Win10Version subVer)
{
    return (QOperatingSystemVersion::current()
            >= ((subVer == Win10Version::Windows10)
                    ? QOperatingSystemVersion::Windows10
                    : QOperatingSystemVersion(QOperatingSystemVersion::Windows,
                                              10,
                                              0,
                                              static_cast<int>(subVer))));
}

void *Widget::rawHandle() const
{
    return reinterpret_cast<void *>(winId());
}

bool Widget::eventFilter(QObject *object, QEvent *event)
{
    Q_ASSERT(object);
    Q_ASSERT(event);
    if (object == this) {
        switch (event->type()) {
        case QEvent::WindowStateChange: {
            if (shouldDrawBorder(true)) {
                if (isMaximized()) {
                    layout()->setContentsMargins(0, 0, 0, 0);
                }
                if (isNormaled()) {
                    layout()->setContentsMargins(1, 1, 1, 1);
                }
            }
            updateTitleBar();
            moveCenterButton->setEnabled(isNormaled());
            break;
        }
        case QEvent::WinIdChange:
            WinNativeEventFilter::addFramelessWindow(this);
            break;
        case QEvent::WindowActivate:
        case QEvent::WindowDeactivate:
            updateTitleBar();
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(object, event);
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
bool Widget::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
#else
bool Widget::nativeEvent(const QByteArray &eventType, void *message, long *result)
#endif
{
    Q_ASSERT(eventType == "windows_generic_MSG");
    Q_ASSERT(message);
    Q_ASSERT(result);
    if (customizeTitleBarCB && customizeTitleBarCB->isChecked()) {
        const auto msg = static_cast<LPMSG>(message);
        switch (msg->message) {
        case WM_NCRBUTTONUP: {
            if (msg->wParam == HTCAPTION) {
                const int x = GET_X_LPARAM(msg->lParam);
                const int y = GET_Y_LPARAM(msg->lParam);
                if (WinNativeEventFilter::displaySystemMenu(msg->hwnd, false, x, y)) {
                    *result = 0;
                    return true;
                }
            }
            break;
        }
        case WM_DWMCOLORIZATIONCOLORCHANGED: {
            g_cColorizationColor = QColor::fromRgba(msg->wParam);
            if (shouldDrawThemedBorder()) {
                updateWindow();
            }
            break;
        }
        case WM_DPICHANGED:
            updateWindow();
            break;
        case WM_NCPAINT:
            update();
            break;
        default:
            break;
        }
    }
    return QWidget::nativeEvent(eventType, message, result);
}

void Widget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    if (shouldDrawBorder()) {
        QPainter painter(this);
        painter.save();
        painter.setPen({borderColor(), 2.0});
        painter.drawLine(0, 0, width(), 0);
        painter.drawLine(0, height(), width(), height());
        painter.drawLine(0, 0, 0, height());
        painter.drawLine(width(), 0, width(), height());
        painter.restore();
    }
}

void Widget::updateWindow()
{
    WinNativeEventFilter::updateFrameMargins(rawHandle());
    WinNativeEventFilter::updateWindow(rawHandle(), true, true);
    update();
}

void Widget::updateTitleBar()
{
    const bool themedTitleBar = shouldDrawThemedTitleBar() && isActiveWindow();
    if (themedTitleBar && !m_bExtendToTitleBar) {
        minimizeButton->setIcon(QIcon(g_sMinimizeButtonImageLight));
        closeButton->setIcon(QIcon(g_sCloseButtonImageLight));
        if (isMaximized()) {
            maximizeButton->setIcon(QIcon(g_sRestoreButtonImageLight));
        }
        if (isNormaled()) {
            maximizeButton->setIcon(QIcon(g_sMaximizeButtonImageLight));
        }
    } else {
        minimizeButton->setIcon(QIcon(g_sMinimizeButtonImageDark));
        closeButton->setIcon(QIcon(g_sCloseButtonImageDark));
        if (isMaximized()) {
            maximizeButton->setIcon(QIcon(g_sRestoreButtonImageDark));
        }
        if (isNormaled()) {
            maximizeButton->setIcon(QIcon(g_sMaximizeButtonImageDark));
        }
    }
    const QColor titleBarBackgroundColor = m_bExtendToTitleBar
                                               ? Qt::transparent
                                               : (themedTitleBar ? g_cColorizationColor : Qt::white);
    const QColor titleBarTextColor = isActiveWindow()
                                         ? ((!themedTitleBar || m_bExtendToTitleBar) ? Qt::black
                                                                                     : Qt::white)
                                         : QColor("#999999");
    const QColor titleBarBorderColor = (!m_bIsWin10OrGreater || shouldDrawBorder() || isMaximized()
                                        || isFullScreen())
                                           ? Qt::transparent
                                           : borderColor();
    titleBarWidget->setStyleSheet(
        g_sSystemButtonsStyleSheet
        + g_sTitleLabelStyleSheet.arg(QString::number(titleBarTextColor.red()),
                                      QString::number(titleBarTextColor.green()),
                                      QString::number(titleBarTextColor.blue()))
        + g_sTitleBarStyleSheet.arg(QString::number(titleBarBackgroundColor.red()),
                                    QString::number(titleBarBackgroundColor.green()),
                                    QString::number(titleBarBackgroundColor.blue()),
                                    QString::number(titleBarBackgroundColor.alpha()),
                                    QString::number(titleBarBorderColor.red()),
                                    QString::number(titleBarBorderColor.green()),
                                    QString::number(titleBarBorderColor.blue()),
                                    QString::number(titleBarBorderColor.alpha())));
}

void Widget::initializeOptions()
{
    if (m_bIsWin10OrGreater) {
        //preserveWindowFrameCB->click();
        if (m_bCanAcrylicBeEnabled) {
            forceAcrylicCB->click();
        }
    }
    customizeTitleBarCB->click();
    extendToTitleBarCB->click();
    blurEffectCB->click();
    resizableCB->click();
    m_bCanShowColorDialog = true;
}

void Widget::setupConnections()
{
    connect(iconButton, &QPushButton::clicked, this, [this]() {
        POINT pos = {};
        GetCursorPos(&pos);
        const auto hwnd = reinterpret_cast<HWND>(rawHandle());
        SendMessageW(hwnd, WM_CONTEXTMENU, reinterpret_cast<WPARAM>(hwnd), MAKELPARAM(pos.x, pos.y));
    });
    connect(minimizeButton, &QPushButton::clicked, this, &Widget::showMinimized);
    connect(maximizeButton, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(closeButton, &QPushButton::clicked, this, &Widget::close);
    connect(moveCenterButton, &QPushButton::clicked, this, [this]() {
        WinNativeEventFilter::moveWindowToDesktopCenter(rawHandle());
    });
    connect(this, &Widget::windowTitleChanged, titleLabel, &QLabel::setText);
    connect(this, &Widget::windowIconChanged, iconButton, &QPushButton::setIcon);
    connect(customizeTitleBarCB, &QCheckBox::stateChanged, this, [this](int state) {
        const bool enable = state == Qt::Checked;
        preserveWindowFrameCB->setEnabled(enable);
        WinNativeEventFilter::updateQtFrame(windowHandle(),
                                            enable ? WinNativeEventFilter::getSystemMetric(
                                                rawHandle(),
                                                WinNativeEventFilter::SystemMetric::TitleBarHeight)
                                                   : 0);
        titleBarWidget->setVisible(enable);
        if (enable) {
            qunsetenv(g_sUseNativeTitleBar);
        } else {
            qputenv(g_sUseNativeTitleBar, "1");
        }
        updateWindow();
    });
    connect(preserveWindowFrameCB, &QCheckBox::stateChanged, this, [this](int state) {
        const bool enable = state == Qt::Checked;
        if (enable) {
            qputenv(g_sPreserveWindowFrame, "1");
            qputenv(g_sDontExtendFrame, "1");
        } else {
            qunsetenv(g_sPreserveWindowFrame);
            qunsetenv(g_sDontExtendFrame);
        }
        if (!enable && shouldDrawBorder()) {
            layout()->setContentsMargins(1, 1, 1, 1);
        } else {
            layout()->setContentsMargins(0, 0, 0, 0);
        }
        updateTitleBar();
        updateWindow();
    });
    connect(blurEffectCB, &QCheckBox::stateChanged, this, [this](int state) {
        const bool enable = state == Qt::Checked;
        QColor color = {0, 0, 0, 127};
        const bool useAcrylicEffect = m_bCanAcrylicBeEnabled && forceAcrylicCB->isChecked();
        if (useAcrylicEffect) {
            if (enable && m_bCanShowColorDialog) {
                color = QColorDialog::getColor(color,
                                               this,
                                               tr("Please select a gradient color"),
                                               QColorDialog::ShowAlphaChannel);
            }
        }
        WinNativeEventFilter::setBlurEffectEnabled(rawHandle(), enable, color);
        updateWindow();
        if (useAcrylicEffect && enable && WinNativeEventFilter::transparencyEffectEnabled()) {
            QMessageBox::warning(this,
                                 tr("BUG Warning!"),
                                 tr("You have enabled the transparency effect in the personalize "
                                    "settings.\nDragging will be very laggy when the Acrylic "
                                    "effect is enabled.\nDisabling the transparency effect can "
                                    "solve this issue temporarily."));
        }
    });
    connect(extendToTitleBarCB, &QCheckBox::stateChanged, this, [this](int state) {
        m_bExtendToTitleBar = state == Qt::Checked;
        updateTitleBar();
    });
    connect(forceAcrylicCB, &QCheckBox::stateChanged, this, [this](int state) {
        if (!m_bCanAcrylicBeEnabled) {
            return;
        }
        if (state == Qt::Checked) {
            qputenv(g_sForceUseAcrylicEffect, "1");
        } else {
            qunsetenv(g_sForceUseAcrylicEffect);
        }
        if (blurEffectCB->isChecked()) {
            blurEffectCB->click();
            blurEffectCB->click();
        }
    });
    connect(resizableCB, &QCheckBox::stateChanged, this, [this](int state) {
        const bool enable = state == Qt::Checked;
        maximizeButton->setEnabled(enable);
        WinNativeEventFilter::setWindowResizable(rawHandle(), enable);
    });
}

void Widget::initializeFramelessFunctions()
{
    WinNativeEventFilter::WINDOWDATA data = {};
    data.ignoreObjects << iconButton << minimizeButton << maximizeButton << closeButton;
    WinNativeEventFilter::addFramelessWindow(this, &data);
    installEventFilter(this);
}

void Widget::initializeVariables()
{
    m_bIsWin10OrGreater = isWin10OrGreater();
    if (m_bIsWin10OrGreater) {
        m_bCanAcrylicBeEnabled = isWin10OrGreater(g_vAcrylicEffectVersion);
        g_cColorizationColor = WinNativeEventFilter::colorizationColor();
    }
}

void Widget::initializeWindow()
{
    initializeVariables();
    setupUi();
    updateTitleBar();
    initializeFramelessFunctions();
    setupConnections();
    initializeOptions();
}
