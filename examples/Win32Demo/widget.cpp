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
#include <QColorDialog>
#include <QDebug>
#include <QOperatingSystemVersion>
#include <QStyleOption>
#include <qt_windows.h>

// Copied from windowsx.h
#define GET_X_LPARAM(lp) ((int) (short) LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int) (short) HIWORD(lp))

namespace {

const char useNativeTitleBar[] = "WNEF_USE_NATIVE_TITLE_BAR";
const char preserveWindowFrame[] = "WNEF_FORCE_PRESERVE_WINDOW_FRAME";
const char forceUseAcrylicEffect[] = "WNEF_FORCE_ACRYLIC_ON_WIN10";

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
        WinNativeEventFilter::updateFrameMargins(handle);
        WinNativeEventFilter::updateWindow(handle, true, true);
    }
}

bool isGreaterThanWin10_1803()
{
    return QOperatingSystemVersion::current()
           >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, 17134);
}

} // namespace

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->forceAcrylicCB->setEnabled(isGreaterThanWin10_1803());

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
        ui->windowFrameCB->setEnabled(enable);
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
    connect(ui->windowFrameCB, &QCheckBox::stateChanged, this, [this](int state) {
        if (state == Qt::Checked) {
            qunsetenv(preserveWindowFrame);
        } else {
            qputenv(preserveWindowFrame, "1");
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

    QStyleOption option;
    option.initFrom(this);
    setWindowIcon(style()->standardIcon(QStyle::SP_ComputerIcon, &option));
    setWindowTitle(tr("Hello, World!"));

    WinNativeEventFilter::WINDOWDATA data = {};
    data.ignoreObjects << ui->minimizeButton << ui->maximizeButton << ui->closeButton;
    WinNativeEventFilter::addFramelessWindow(this, &data);

    installEventFilter(this);
}

Widget::~Widget()
{
    delete ui;
}

bool Widget::eventFilter(QObject *object, QEvent *event)
{
    Q_ASSERT(object);
    Q_ASSERT(event);
    switch (event->type()) {
    case QEvent::WindowStateChange: {
        if (isMaximized()) {
            ui->maximizeButton->setIcon(QIcon(QLatin1String(":/images/button_restore_black.svg")));
        } else if (!isFullScreen() && !isMinimized()) {
            ui->maximizeButton->setIcon(QIcon(QLatin1String(":/images/button_maximize_black.svg")));
        }
        ui->moveCenterButton->setEnabled(!isMaximized() && !isFullScreen());
    } break;
    case QEvent::WinIdChange:
        WinNativeEventFilter::addFramelessWindow(this);
        break;
    default:
        break;
    }
    return QWidget::eventFilter(object, event);
}

bool Widget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_ASSERT(eventType == "windows_generic_MSG");
    Q_ASSERT(message);
    Q_ASSERT(result);
    if (ui->customizeTitleBarCB->isChecked()) {
        const auto msg = static_cast<LPMSG>(message);
        const auto getCursorPosition = [](const LPARAM lParam) -> QPoint {
            return {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        };
        switch (msg->message) {
        case WM_NCRBUTTONUP: {
            const QPoint pos = getCursorPosition(msg->lParam);
            qDebug() << "WM_NCRBUTTONUP -->" << pos;
            if (WinNativeEventFilter::displaySystemMenu(msg->hwnd, false, pos.x(), pos.y())) {
                *result = 0;
                return true;
            }
            break;
        }
        case WM_RBUTTONUP:
            qDebug() << "WM_RBUTTONUP -->" << getCursorPosition(msg->lParam);
            break;
        default:
            break;
        }
    }
    return QWidget::nativeEvent(eventType, message, result);
}
