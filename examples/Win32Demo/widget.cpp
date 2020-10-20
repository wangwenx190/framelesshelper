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

const char useNativeTitleBar[] = "WNEF_USE_NATIVE_TITLE_BAR";
const char preserveWindowFrame[] = "WNEF_FORCE_PRESERVE_WINDOW_FRAME";
const char forceUseAcrylicEffect[] = "WNEF_FORCE_ACRYLIC_ON_WIN10";

const QLatin1String systemButtonsStyleSheet(R"(
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

void *getRawHandle(QWidget *widget)
{
    Q_ASSERT(widget);
    return widget->isTopLevel() ? reinterpret_cast<void *>(widget->winId()) : nullptr;
}

void updateWindow(QWidget *widget)
{
    Q_ASSERT(widget);
    if (widget->isTopLevel()) {
        void *handle = getRawHandle(widget);
        //WinNativeEventFilter::updateFrameMargins(handle);
        WinNativeEventFilter::updateWindow(handle, true, true);
        widget->update();
    }
}

bool isWin10OrGreater()
{
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10;
}

bool isGreaterThanWin10_1803()
{
    return QOperatingSystemVersion::current()
           >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, 17134);
}

bool isThemeColorEnabled()
{
    if (!isWin10OrGreater()) {
        return false;
    }
    bool ok = false;
    const QSettings registry(QLatin1String(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\DWM)"),
                             QSettings::NativeFormat);
    const bool colorPrevalence = registry.value(QLatin1String("ColorPrevalence"), 0).toULongLong(&ok)
                                 != 0;
    return (ok && colorPrevalence);
}

bool isDarkModeEnabled()
{
    if (!isWin10OrGreater()) {
        return false;
    }
    bool ok = false;
    const QSettings registry(
        QLatin1String(
            R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)"),
        QSettings::NativeFormat);
    const bool appsUseLightTheme
        = registry.value(QLatin1String("AppsUseLightTheme"), 0).toULongLong(&ok) != 0;
    return (ok && !appsUseLightTheme);
}

QColor getThemeColor()
{
    DWORD color = 0;
    BOOL opaqueBlend = FALSE;
    return SUCCEEDED(DwmGetColorizationColor(&color, &opaqueBlend)) ? QColor::fromRgba(color)
                                                                    : Qt::white;
}

} // namespace

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    m_bIsWin10OrGreater = isWin10OrGreater();

    m_cThemeColor = getThemeColor();

    ui->setupUi(this);

    ui->forceAcrylicCB->setEnabled(isGreaterThanWin10_1803());

    if (shouldDrawBorder()) {
        layout()->setContentsMargins(1, 1, 1, 1);
    }

    updateTitleBar();

    connect(ui->iconButton, &QPushButton::clicked, this, [this]() {
        POINT pos = {};
        GetCursorPos(&pos);
        const auto hwnd = reinterpret_cast<HWND>(getRawHandle(this));
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
        WinNativeEventFilter::moveWindowToDesktopCenter(getRawHandle(this));
    });

    connect(this, &Widget::windowTitleChanged, ui->titleLabel, &QLabel::setText);
    connect(this, &Widget::windowIconChanged, ui->iconButton, &QPushButton::setIcon);

    connect(ui->customizeTitleBarCB, &QCheckBox::stateChanged, this, [this](int state) {
        const bool enable = state == Qt::Checked;
        ui->removeWindowFrameCB->setEnabled(enable);
        WinNativeEventFilter::updateQtFrame(windowHandle(),
                                            enable ? WinNativeEventFilter::getSystemMetric(
                                                getRawHandle(this),
                                                WinNativeEventFilter::SystemMetric::TitleBarHeight)
                                                   : 0);
        ui->titleBarWidget->setVisible(enable);
        if (enable) {
            qunsetenv(useNativeTitleBar);
        } else {
            qputenv(useNativeTitleBar, "1");
        }
        updateWindow(this);
    });
    connect(ui->removeWindowFrameCB, &QCheckBox::stateChanged, this, [this](int state) {
        const bool enable = state == Qt::Checked;
        if (enable) {
            qunsetenv(preserveWindowFrame);
        } else {
            qputenv(preserveWindowFrame, "1");
        }
        if (enable && shouldDrawBorder()) {
            layout()->setContentsMargins(1, 1, 1, 1);
        } else {
            layout()->setContentsMargins(0, 0, 0, 0);
        }
        updateWindow(this);
    });
    connect(ui->blurEffectCB, &QCheckBox::stateChanged, this, [this](int state) {
        const bool enable = state == Qt::Checked;
        QColor color = QColor(255, 255, 255, 0);
        if (isGreaterThanWin10_1803() && ui->forceAcrylicCB->isChecked()) {
            if (enable) {
                color = QColorDialog::getColor(color,
                                               this,
                                               tr("Please select a gradient color"),
                                               QColorDialog::ShowAlphaChannel);
            }
        }
        WinNativeEventFilter::setBlurEffectEnabled(getRawHandle(this), enable, color);
        updateWindow(this);
    });
    connect(ui->extendToTitleBarCB, &QCheckBox::stateChanged, this, [this](int state) {
        m_bExtendToTitleBar = state == Qt::Checked;
        updateTitleBar();
    });
    connect(ui->forceAcrylicCB, &QCheckBox::stateChanged, this, [this](int state) {
        if (state == Qt::Checked) {
            qputenv(forceUseAcrylicEffect, "1");
        } else {
            qunsetenv(forceUseAcrylicEffect);
        }
        if (ui->blurEffectCB->isChecked()) {
            ui->blurEffectCB->click();
            ui->blurEffectCB->click();
        }
    });
    connect(ui->resizableCB, &QCheckBox::stateChanged, this, [this](int state) {
        const bool enable = state == Qt::Checked;
        ui->maximizeButton->setEnabled(enable);
        WinNativeEventFilter::setWindowResizable(getRawHandle(this), enable);
    });

    WinNativeEventFilter::WINDOWDATA data = {};
    data.ignoreObjects << ui->iconButton << ui->minimizeButton << ui->maximizeButton
                       << ui->closeButton;
    WinNativeEventFilter::addFramelessWindow(this, &data);

    installEventFilter(this);
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
           && ui->removeWindowFrameCB->isChecked() && ui->customizeTitleBarCB->isChecked();
}

bool Widget::shouldDrawThemedBorder(const bool ignoreWindowState) const
{
    return (shouldDrawBorder(ignoreWindowState) && isThemeColorEnabled());
}

QColor Widget::activeBorderColor() const
{
    return isThemeColorEnabled() ? m_cThemeColor
                                 : (isDarkModeEnabled() ? m_cDefaultActiveBorderColor : Qt::white);
}

QColor Widget::inactiveBorderColor() const
{
    return m_cDefaultInactiveBorderColor;
}

QColor Widget::borderColor() const
{
    return isActiveWindow() ? activeBorderColor() : inactiveBorderColor();
}

bool Widget::eventFilter(QObject *object, QEvent *event)
{
    Q_ASSERT(object);
    Q_ASSERT(event);
    Q_ASSERT(object == this);
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
            m_cThemeColor = QColor::fromRgba(msg->wParam);
            if (shouldDrawThemedBorder()) {
                updateWindow(this);
            }
            break;
        }
        case WM_DPICHANGED:
            updateWindow(this);
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

void Widget::updateTitleBar()
{
    const bool themedTitleBar = shouldDrawThemedBorder(true) && isActiveWindow();
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
    const QColor titleBarColor = m_bExtendToTitleBar ? Qt::transparent
                                                     : (themedTitleBar ? m_cThemeColor : Qt::white);
    const QColor titleTextColor = isActiveWindow()
                                      ? ((!themedTitleBar || m_bExtendToTitleBar) ? Qt::black
                                                                                  : Qt::white)
                                      : QColor("#999999");
    ui->titleBarWidget->setStyleSheet(systemButtonsStyleSheet
                                      + QLatin1String(R"(
#titleLabel {
  color: rgb(%1, %2, %3);
}
)")
                                            .arg(QString::number(titleTextColor.red()),
                                                 QString::number(titleTextColor.green()),
                                                 QString::number(titleTextColor.blue()))
                                      + QLatin1String(R"(
#titleBarWidget {
  background-color: rgba(%1, %2, %3, %4);
}
)")
                                            .arg(QString::number(titleBarColor.red()),
                                                 QString::number(titleBarColor.green()),
                                                 QString::number(titleBarColor.blue()),
                                                 QString::number(titleBarColor.alpha())));
}
