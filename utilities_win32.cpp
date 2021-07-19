/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
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

#include "utilities.h"
#include <QtCore/qt_windows.h>
#include <QtGui/qguiapplication.h>
#include <QtCore/qdebug.h>
#include <dwmapi.h>
#include <QtGui/qpa/qplatformwindow.h>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QtGui/qpa/qplatformnativeinterface.h>
#else
#include <QtGui/qpa/qplatformwindow_p.h>
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
#include <QtCore/qoperatingsystemversion.h>
#else
#include <QtCore/qsysinfo.h>
#endif

Q_DECLARE_METATYPE(QMargins)

#ifndef SM_CXPADDEDBORDER
// Only available since Windows Vista
#define SM_CXPADDEDBORDER 92
#endif

// The standard values of resize border width, resize border height and title bar height when DPI is 96.
static const int g_defaultResizeBorderWidth = 8, g_defaultResizeBorderHeight = 8, g_defaultTitleBarHeight = 31;

bool Utilities::isDwmCompositionAvailable()
{
    // DWM Composition is always enabled and can't be disabled since Windows 8.
    if (isWin8OrGreater()) {
        return true;
    }
    BOOL enabled = FALSE;
    if (FAILED(DwmIsCompositionEnabled(&enabled))) {
        qWarning() << "DwmIsCompositionEnabled() failed.";
        return false;
    }
    return (enabled != FALSE);
}

int Utilities::getSystemMetric(const QWindow *window, const SystemMetric metric, const bool dpiScale, const bool forceSystemValue)
{
    Q_ASSERT(window);
    if (!window) {
        return 0;
    }
    switch (metric) {
    case SystemMetric::ResizeBorderWidth: {
        const int bw = window->property(_flh_global::_flh_borderWidth_flag).toInt();
        if ((bw > 0) && !forceSystemValue) {
            return qRound(static_cast<qreal>(bw) * (dpiScale ? window->devicePixelRatio() : 1.0));
        } else {
            const int result = GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
            if (result > 0) {
                if (dpiScale) {
                    return result;
                } else {
                    return qRound(static_cast<qreal>(result) / window->devicePixelRatio());
                }
            } else {
                if (dpiScale) {
                    return qRound(static_cast<qreal>(g_defaultResizeBorderWidth) * window->devicePixelRatio());
                } else {
                    return g_defaultResizeBorderWidth;
                }
            }
        }
    }
    case SystemMetric::ResizeBorderHeight: {
        const int bh = window->property(_flh_global::_flh_borderHeight_flag).toInt();
        if ((bh > 0) && !forceSystemValue) {
            return qRound(static_cast<qreal>(bh) * (dpiScale ? window->devicePixelRatio() : 1.0));
        } else {
            // There is no "SM_CYPADDEDBORDER".
            const int result = GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
            if (result > 0) {
                if (dpiScale) {
                    return result;
                } else {
                    return qRound(static_cast<qreal>(result) / window->devicePixelRatio());
                }
            } else {
                if (dpiScale) {
                    return qRound(static_cast<qreal>(g_defaultResizeBorderHeight) * window->devicePixelRatio());
                } else {
                    return g_defaultResizeBorderHeight;
                }
            }
        }
    }
    case SystemMetric::TitleBarHeight: {
        const int tbh = window->property(_flh_global::_flh_titleBarHeight_flag).toInt();
        if ((tbh > 0) && !forceSystemValue) {
            return qRound(static_cast<qreal>(tbh) * (dpiScale ? window->devicePixelRatio() : 1.0));
        } else {
            // There is no "SM_CYPADDEDBORDER".
            const int result = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
            if (result > 0) {
                if (dpiScale) {
                    return result;
                } else {
                    return qRound(static_cast<qreal>(result) / window->devicePixelRatio());
                }
            } else {
                if (dpiScale) {
                    return qRound(static_cast<qreal>(g_defaultTitleBarHeight) * window->devicePixelRatio());
                } else {
                    return g_defaultTitleBarHeight;
                }
            }
        }
    }
    }
    return 0;
}

void Utilities::triggerFrameChange(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    if (SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER) == FALSE) {
        qWarning() << "SetWindowPos() failed.";
    }
}

void Utilities::updateFrameMargins(const QWindow *window, const bool reset)
{
    // DwmExtendFrameIntoClientArea() will always fail if DWM Composition is disabled.
    // No need to try in this case.
    if (!isDwmCompositionAvailable()) {
        return;
    }
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    const MARGINS margins = reset ? MARGINS{0, 0, 0, 0} : MARGINS{1, 1, 1, 1};
    if (FAILED(DwmExtendFrameIntoClientArea(hwnd, &margins))) {
        qWarning() << "DwmExtendFrameIntoClientArea() failed.";
    }
}

void Utilities::updateQtFrameMargins(QWindow *window, const bool enable)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    const int tbh = enable ? Utilities::getSystemMetric(window, Utilities::SystemMetric::TitleBarHeight, true, true) : 0;
    const int bw = enable ? Utilities::getSystemMetric(window, Utilities::SystemMetric::ResizeBorderWidth, true, true) : 0;
    const int bh = enable ? Utilities::getSystemMetric(window, Utilities::SystemMetric::ResizeBorderHeight, true, true) : 0;
    const QMargins margins = {-bw, -tbh, -bw, -bh}; // left, top, right, bottom
    const QVariant marginsVar = QVariant::fromValue(margins);
    window->setProperty("_q_windowsCustomMargins", marginsVar);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QPlatformWindow *platformWindow = window->handle();
    if (platformWindow) {
        QGuiApplication::platformNativeInterface()->setWindowProperty(platformWindow, QStringLiteral("WindowsCustomMargins"), marginsVar);
    }
#else
    auto *platformWindow = dynamic_cast<QNativeInterface::Private::QWindowsWindow *>(
        window->handle());
    if (platformWindow) {
        platformWindow->setCustomMargins(margins);
    }
#endif
}

bool Utilities::isWin8OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8;
#else
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8;
#endif
}

bool Utilities::isWin8Point1OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8_1;
#else
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8_1;
#endif
}

bool Utilities::isWin10OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10;
#else
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10;
#endif
}
