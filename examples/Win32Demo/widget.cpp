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
#include "ui_widget.h"
#include <dwmapi.h>
#include <QColorDialog>
#include <QMessageBox>
#include <QOperatingSystemVersion>
#include <QPainter>
#include <QSettings>
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

enum : WORD { DwmwaUseImmersiveDarkMode = 20, DwmwaUseImmersiveDarkModeBefore20h1 = 19 };

const Widget::Win10Version g_vAcrylicEffectVersion = Widget::Win10Version::Win10_1803;
const Widget::Win10Version g_vLightThemeVersion = Widget::Win10Version::Win10_1809;
const Widget::Win10Version g_vDarkThemeVersion = g_vLightThemeVersion; // check
const Widget::Win10Version g_vDarkFrameVersion = g_vDarkThemeVersion;  // check

const QColor g_cDefaultActiveBorderColor = {"#707070"};
const QColor g_cDefaultInactiveBorderColor = {"#aaaaaa"};
QColor g_cColorizationColor = Qt::white;

const char g_sUseNativeTitleBar[] = "WNEF_USE_NATIVE_TITLE_BAR";
const char g_sPreserveWindowFrame[] = "WNEF_FORCE_PRESERVE_WINDOW_FRAME";
const char g_sForceUseAcrylicEffect[] = "WNEF_FORCE_ACRYLIC_ON_WIN10";
const char g_sDontExtendFrame[] = "WNEF_DO_NOT_EXTEND_FRAME";

const QLatin1String g_sDwmRegistryKey(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\DWM)");
const QLatin1String g_sPersonalizeRegistryKey(
    R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)");

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

} // namespace

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    m_bIsWin10OrGreater = isWin10OrGreater();
    m_bIsWin10_1803OrGreater = isWin10OrGreater(g_vAcrylicEffectVersion);
    g_cColorizationColor = colorizationColor();

    ui->setupUi(this);
    ui->forceAcrylicCB->setEnabled(m_bIsWin10_1803OrGreater);
    if (shouldDrawBorder()) {
        layout()->setContentsMargins(1, 1, 1, 1);
    } else {
        layout()->setContentsMargins(0, 0, 0, 0);
    }
    updateTitleBar();

    connect(ui->iconButton, &QPushButton::clicked, this, [this]() {
        POINT pos = {};
        GetCursorPos(&pos);
        const auto hwnd = reinterpret_cast<HWND>(rawHandle());
        SendMessageW(hwnd, WM_CONTEXTMENU, reinterpret_cast<WPARAM>(hwnd), MAKELPARAM(pos.x, pos.y));
    });
    connect(ui->minimizeButton, &QPushButton::clicked, this, &Widget::showMinimized);
    connect(ui->maximizeButton, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(ui->closeButton, &QPushButton::clicked, this, &Widget::close);
    connect(ui->moveCenterButton, &QPushButton::clicked, this, [this]() {
        WinNativeEventFilter::moveWindowToDesktopCenter(rawHandle());
    });
    connect(this, &Widget::windowTitleChanged, ui->titleLabel, &QLabel::setText);
    connect(this, &Widget::windowIconChanged, ui->iconButton, &QPushButton::setIcon);
    connect(ui->customizeTitleBarCB, &QCheckBox::stateChanged, this, [this](int state) {
        const bool enable = state == Qt::Checked;
        ui->preserveWindowFrameCB->setEnabled(enable);
        WinNativeEventFilter::updateQtFrame(windowHandle(),
                                            enable ? WinNativeEventFilter::getSystemMetric(
                                                rawHandle(),
                                                WinNativeEventFilter::SystemMetric::TitleBarHeight)
                                                   : 0);
        ui->titleBarWidget->setVisible(enable);
        if (enable) {
            qunsetenv(g_sUseNativeTitleBar);
        } else {
            qputenv(g_sUseNativeTitleBar, "1");
        }
        updateWindow();
    });
    connect(ui->preserveWindowFrameCB, &QCheckBox::stateChanged, this, [this](int state) {
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
    connect(ui->blurEffectCB, &QCheckBox::stateChanged, this, [this](int state) {
        const bool enable = state == Qt::Checked;
        QColor color = {0, 0, 0, 127};
        const bool useAcrylicEffect = m_bIsWin10_1803OrGreater && ui->forceAcrylicCB->isChecked();
        if (useAcrylicEffect) {
            if (enable && m_bShowColorDialog) {
                color = QColorDialog::getColor(color,
                                               this,
                                               tr("Please select a gradient color"),
                                               QColorDialog::ShowAlphaChannel);
            }
        }
        WinNativeEventFilter::setBlurEffectEnabled(rawHandle(), enable, color);
        updateWindow();
        if (useAcrylicEffect && enable && transparencyEffectEnabled()) {
            QMessageBox::warning(this,
                                 tr("BUG Warning!"),
                                 tr("You have enabled the transparency effect in the personalize "
                                    "settings.\nDragging will be very laggy when the Acrylic "
                                    "effect is enabled.\nDisabling the transparency effect can "
                                    "solve this issue temporarily."));
        }
    });
    connect(ui->extendToTitleBarCB, &QCheckBox::stateChanged, this, [this](int state) {
        m_bExtendToTitleBar = state == Qt::Checked;
        updateTitleBar();
    });
    connect(ui->forceAcrylicCB, &QCheckBox::stateChanged, this, [this](int state) {
        if (!m_bIsWin10_1803OrGreater) {
            return;
        }
        if (state == Qt::Checked) {
            qputenv(g_sForceUseAcrylicEffect, "1");
        } else {
            qunsetenv(g_sForceUseAcrylicEffect);
        }
        if (ui->blurEffectCB->isChecked()) {
            ui->blurEffectCB->click();
            ui->blurEffectCB->click();
        }
    });
    connect(ui->resizableCB, &QCheckBox::stateChanged, this, [this](int state) {
        const bool enable = state == Qt::Checked;
        ui->maximizeButton->setEnabled(enable);
        WinNativeEventFilter::setWindowResizable(rawHandle(), enable);
    });

    WinNativeEventFilter::WINDOWDATA data = {};
    data.ignoreObjects << ui->iconButton << ui->minimizeButton << ui->maximizeButton
                       << ui->closeButton;
    WinNativeEventFilter::addFramelessWindow(this, &data);

    installEventFilter(this);

    initWindow();
}

Widget::~Widget()
{
    delete ui;
}

bool Widget::isNormaled() const
{
    return !isMinimized() && !isMaximized() && !isFullScreen();
}

bool Widget::shouldDrawBorder(const bool ignoreWindowState) const
{
    return m_bIsWin10OrGreater && (ignoreWindowState ? true : isNormaled())
           && !ui->preserveWindowFrameCB->isChecked() && ui->customizeTitleBarCB->isChecked();
}

bool Widget::shouldDrawThemedBorder(const bool ignoreWindowState) const
{
    return (shouldDrawBorder(ignoreWindowState) && colorizationColorEnabled());
}

bool Widget::shouldDrawThemedTitleBar() const
{
    return m_bIsWin10OrGreater && colorizationColorEnabled();
}

QColor Widget::activeBorderColor()
{
    return colorizationColorEnabled() ? g_cColorizationColor : g_cDefaultActiveBorderColor;
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

QColor Widget::colorizationColor()
{
#if 0
    DWORD color = 0;
    BOOL opaqueBlend = FALSE;
    return SUCCEEDED(DwmGetColorizationColor(&color, &opaqueBlend)) ? QColor::fromRgba(color)
                                                                    : Qt::white;
#else
    bool ok = false;
    const QSettings registry(g_sDwmRegistryKey, QSettings::NativeFormat);
    const quint64 color = registry.value(QLatin1String("ColorizationColor"), 0).toULongLong(&ok);
    return ok ? QColor::fromRgba(color) : Qt::white;
#endif
}

bool Widget::colorizationColorEnabled()
{
    if (!isWin10OrGreater()) {
        return false;
    }
    bool ok = false;
    const QSettings registry(g_sDwmRegistryKey, QSettings::NativeFormat);
    const bool colorPrevalence = registry.value(QLatin1String("ColorPrevalence"), 0).toULongLong(&ok)
                                 != 0;
    return (ok && colorPrevalence);
}

bool Widget::lightThemeEnabled()
{
    if (!isWin10OrGreater(g_vLightThemeVersion)) {
        return false;
    }
    bool ok = false;
    const QSettings registry(g_sPersonalizeRegistryKey, QSettings::NativeFormat);
    const bool appsUseLightTheme
        = registry.value(QLatin1String("AppsUseLightTheme"), 0).toULongLong(&ok) != 0;
    return (ok && appsUseLightTheme);
}

bool Widget::darkThemeEnabled()
{
    if (!isWin10OrGreater(g_vDarkThemeVersion)) {
        return false;
    }
    return !lightThemeEnabled();
}

bool Widget::highContrastModeEnabled()
{
    HIGHCONTRASTW hc;
    SecureZeroMemory(&hc, sizeof(hc));
    hc.cbSize = sizeof(hc);
    return SystemParametersInfoW(SPI_GETHIGHCONTRAST, 0, &hc, 0) ? (hc.dwFlags & HCF_HIGHCONTRASTON)
                                                                 : false;
}

bool Widget::darkFrameEnabled(void *handle)
{
    Q_ASSERT(handle);
    if (!isWin10OrGreater(g_vDarkFrameVersion)) {
        return false;
    }
    const auto hwnd = reinterpret_cast<HWND>(handle);
    BOOL result = FALSE;
    const bool ok
        = SUCCEEDED(DwmGetWindowAttribute(hwnd, DwmwaUseImmersiveDarkMode, &result, sizeof(result)))
          || SUCCEEDED(DwmGetWindowAttribute(hwnd,
                                             DwmwaUseImmersiveDarkModeBefore20h1,
                                             &result,
                                             sizeof(result)));
    return (ok && result);
}

bool Widget::transparencyEffectEnabled()
{
    if (!isWin10OrGreater()) {
        return false;
    }
    bool ok = false;
    const QSettings registry(g_sPersonalizeRegistryKey, QSettings::NativeFormat);
    const bool enableTransparency
        = registry.value(QLatin1String("EnableTransparency"), 0).toULongLong(&ok) != 0;
    return (ok && enableTransparency);
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
            ui->moveCenterButton->setEnabled(isNormaled());
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
    if (ui->customizeTitleBarCB->isChecked()) {
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
        ui->minimizeButton->setIcon(QIcon(QLatin1String(":/images/button_minimize_white.svg")));
        ui->closeButton->setIcon(QIcon(QLatin1String(":/images/button_close_white.svg")));
        if (isMaximized()) {
            ui->maximizeButton->setIcon(QIcon(QLatin1String(":/images/button_restore_white.svg")));
        }
        if (isNormaled()) {
            ui->maximizeButton->setIcon(QIcon(QLatin1String(":/images/button_maximize_white.svg")));
        }
    } else {
        ui->minimizeButton->setIcon(QIcon(QLatin1String(":/images/button_minimize_black.svg")));
        ui->closeButton->setIcon(QIcon(QLatin1String(":/images/button_close_black.svg")));
        if (isMaximized()) {
            ui->maximizeButton->setIcon(QIcon(QLatin1String(":/images/button_restore_black.svg")));
        }
        if (isNormaled()) {
            ui->maximizeButton->setIcon(QIcon(QLatin1String(":/images/button_maximize_black.svg")));
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
    ui->titleBarWidget->setStyleSheet(g_sSystemButtonsStyleSheet
                                      + QLatin1String(R"(
#titleLabel {
  color: rgb(%1, %2, %3);
}
)")
                                            .arg(QString::number(titleBarTextColor.red()),
                                                 QString::number(titleBarTextColor.green()),
                                                 QString::number(titleBarTextColor.blue()))
                                      + QLatin1String(R"(
#titleBarWidget {
  background-color: rgba(%1, %2, %3, %4);
  border-top: 1px solid rgba(%5, %6, %7, %8);
}
)")
                                            .arg(QString::number(titleBarBackgroundColor.red()),
                                                 QString::number(titleBarBackgroundColor.green()),
                                                 QString::number(titleBarBackgroundColor.blue()),
                                                 QString::number(titleBarBackgroundColor.alpha()),
                                                 QString::number(titleBarBorderColor.red()),
                                                 QString::number(titleBarBorderColor.green()),
                                                 QString::number(titleBarBorderColor.blue()),
                                                 QString::number(titleBarBorderColor.alpha())));
}

void Widget::initWindow()
{
    if (m_bIsWin10OrGreater) {
        //ui->preserveWindowFrameCB->click();
        if (m_bIsWin10_1803OrGreater) {
            ui->forceAcrylicCB->click();
        }
    }
    ui->customizeTitleBarCB->click();
    ui->extendToTitleBarCB->click();
    ui->blurEffectCB->click();
    ui->resizableCB->click();
    m_bShowColorDialog = true;
}
