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
#include <QtCore/qdebug.h>
#include <QtCore/qsettings.h>
#include <QtCore/qt_windows.h>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
#include <QtCore/qoperatingsystemversion.h>
#else
#include <QtCore/qsysinfo.h>
#endif
#include <QtGui/qguiapplication.h>
#include <QtGui/qpa/qplatformwindow.h>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QtGui/qpa/qplatformnativeinterface.h>
#else
#include <QtGui/qpa/qplatformwindow_p.h>
#endif
#include <dwmapi.h>

Q_DECLARE_METATYPE(QMargins)

#ifndef SM_CXPADDEDBORDER
// Only available since Windows Vista
#define SM_CXPADDEDBORDER (92)
#endif

FRAMELESSHELPER_BEGIN_NAMESPACE

static constexpr char kDwmRegistryKey[] = R"(Software\Microsoft\Windows\DWM)";

static constexpr int kDefaultResizeBorderThicknessClassic = 4;
static constexpr int kDefaultResizeBorderThicknessAero = 8;
static constexpr int kDefaultCaptionHeight = 23;

bool Utilities::isDwmCompositionAvailable()
{
    // DWM Composition is always enabled and can't be disabled since Windows 8.
    if (isWin8OrGreater()) {
        return true;
    }
    BOOL enabled = FALSE;
    if (SUCCEEDED(DwmIsCompositionEnabled(&enabled))) {
        return (enabled != FALSE);
    }
    const QSettings registry(QString::fromUtf8(kDwmRegistryKey), QSettings::NativeFormat);
    bool ok = false;
    const int value = registry.value(QStringLiteral("Composition"), 0).toInt(&ok);
    return (ok && (value != 0));
}

int Utilities::getSystemMetric(const QWindow *window, const SystemMetric metric, const bool dpiScale, const bool forceSystemValue)
{
    Q_ASSERT(window);
    if (!window) {
        return 0;
    }
    const qreal devicePixelRatio = window->devicePixelRatio();
    const qreal scaleFactor = (dpiScale ? devicePixelRatio : 1.0);
    switch (metric) {
    case SystemMetric::ResizeBorderThickness: {
        const int resizeBorderThickness = window->property(Constants::kResizeBorderThicknessFlag).toInt();
        if ((resizeBorderThickness > 0) && !forceSystemValue) {
            return qRound(static_cast<qreal>(resizeBorderThickness) * scaleFactor);
        } else {
            const int result = GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
            if (result > 0) {
                if (dpiScale) {
                    return result;
                } else {
                    return qRound(static_cast<qreal>(result) / devicePixelRatio);
                }
            } else {
                // The padded border will disappear if DWM composition is disabled.
                const int defaultResizeBorderThickness = (isDwmCompositionAvailable() ? kDefaultResizeBorderThicknessAero : kDefaultResizeBorderThicknessClassic);
                if (dpiScale) {
                    return qRound(static_cast<qreal>(defaultResizeBorderThickness) * devicePixelRatio);
                } else {
                    return defaultResizeBorderThickness;
                }
            }
        }
    }
    case SystemMetric::CaptionHeight: {
        const int captionHeight = window->property(Constants::kCaptionHeightFlag).toInt();
        if ((captionHeight > 0) && !forceSystemValue) {
            return qRound(static_cast<qreal>(captionHeight) * scaleFactor);
        } else {
            const int result = GetSystemMetrics(SM_CYCAPTION);
            if (result > 0) {
                if (dpiScale) {
                    return result;
                } else {
                    return qRound(static_cast<qreal>(result) / devicePixelRatio);
                }
            } else {
                if (dpiScale) {
                    return qRound(static_cast<qreal>(kDefaultCaptionHeight) * devicePixelRatio);
                } else {
                    return kDefaultCaptionHeight;
                }
            }
        }
    }
    case SystemMetric::TitleBarHeight: {
        const int titleBarHeight = window->property(Constants::kTitleBarHeightFlag).toInt();
        if ((titleBarHeight > 0) && !forceSystemValue) {
            return qRound(static_cast<qreal>(titleBarHeight) * scaleFactor);
        } else {
            const int captionHeight = getSystemMetric(window,SystemMetric::CaptionHeight,
                                                      dpiScale, forceSystemValue);
            const int resizeBorderThickness = getSystemMetric(window, SystemMetric::ResizeBorderThickness,
                                                              dpiScale, forceSystemValue);
            return (((window->windowState() == Qt::WindowMaximized)
                     || (window->windowState() == Qt::WindowFullScreen))
                    ? captionHeight : (captionHeight + resizeBorderThickness));
        }
    }
    }
    return 0;
}

void Utilities::triggerFrameChange(const WId winId)
{
    Q_ASSERT(winId);
    if (!winId) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(winId);
    constexpr UINT flags = (SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    if (SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, flags) == FALSE) {
        qWarning() << "SetWindowPos() failed.";
    }
}

void Utilities::updateFrameMargins(const WId winId, const bool reset)
{
    // DwmExtendFrameIntoClientArea() will always fail if DWM composition is disabled.
    // No need to try in this case.
    if (!isDwmCompositionAvailable()) {
        return;
    }
    Q_ASSERT(winId);
    if (!winId) {
        return;
    }
    const auto hwnd = reinterpret_cast<HWND>(winId);
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
    const bool shouldApplyCustomFrameMargins = (enable
                                                && (window->windowState() != Qt::WindowMaximized)
                                                && (window->windowState() != Qt::WindowFullScreen));
    const int resizeBorderThickness = shouldApplyCustomFrameMargins ? Utilities::getSystemMetric(window, SystemMetric::ResizeBorderThickness, true, true) : 0;
    const int titleBarHeight = shouldApplyCustomFrameMargins ? Utilities::getSystemMetric(window, SystemMetric::TitleBarHeight, true, true) : 0;
    const QMargins margins = {-resizeBorderThickness, -titleBarHeight, -resizeBorderThickness, -resizeBorderThickness}; // left, top, right, bottom
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
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8);
#else
    static const bool result = (QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8);
#endif
    return result;
}

bool Utilities::isWin8Point1OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8_1);
#else
    static const bool result = (QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8_1);
#endif
    return result;
}

bool Utilities::isWin10OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10);
#else
    static const bool result = (QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10);
#endif
    return result;
}

FRAMELESSHELPER_END_NAMESPACE
