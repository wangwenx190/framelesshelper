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

#include "framelesshelper_global.h"
#include <QAbstractNativeEventFilter>
#include <QColor>
#include <QList>
#include <QRect>

QT_BEGIN_NAMESPACE
QT_FORWARD_DECLARE_CLASS(QWindow)
QT_FORWARD_DECLARE_CLASS(QObject)
QT_END_NAMESPACE

#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
#define Q_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;

#define Q_DISABLE_COPY_MOVE(Class) \
    Q_DISABLE_COPY(Class) \
    Q_DISABLE_MOVE(Class)
#endif

class FRAMELESSHELPER_EXPORT WinNativeEventFilter : public QAbstractNativeEventFilter
{
    Q_DISABLE_COPY_MOVE(WinNativeEventFilter)

public:
    using WINDOWDATA = struct _WINDOWDATA
    {
        bool initialized = false /* Internal use only, don't modify it from outside */,
             fixedSize = false, mouseTransparent = false, restoreDefaultWindowStyle = false,
             enableLayeredWindow = false, disableTitleBar = false, enableBlurBehindWindow = false;
        int borderWidth = -1, borderHeight = -1, titleBarHeight = -1;
        QList<QRect> ignoreAreas = {}, draggableAreas = {};
        QList<QObject *> ignoreObjects = {}, draggableObjects = {};
        QSize maximumSize = {}, minimumSize = {};
        QString currentScreen = {};
    };

    enum class SystemMetric { BorderWidth, BorderHeight, TitleBarHeight };

    explicit WinNativeEventFilter();
    ~WinNativeEventFilter() override = default;

    // Make the given window become frameless.
    // The width and height will be scaled automatically according to DPI. Don't
    // scale them yourself. Just pass the original value. If you don't want to
    // change them, pass negative values to the parameters.
    static void addFramelessWindow(void *window /* HWND */,
                                   const WINDOWDATA *data = nullptr,
                                   const bool center = false,
                                   const int x = -1,
                                   const int y = -1,
                                   const int width = -1,
                                   const int height = -1);
    static void addFramelessWindow(QObject *window,
                                   const WINDOWDATA *data = nullptr,
                                   const bool center = false,
                                   const int x = -1,
                                   const int y = -1,
                                   const int width = -1,
                                   const int height = -1);
    static void removeFramelessWindow(void *window /* HWND */);
    static void removeFramelessWindow(QObject *window);
    static void clearFramelessWindows();

    // Set borderWidth, borderHeight or titleBarHeight to a negative value to
    // restore default behavior.
    // Note that it can only affect one specific window.
    // If you want to change these values globally, use setBorderWidth instead.
    static void setWindowData(void *window /* HWND */, const WINDOWDATA *data);
    static void setWindowData(QObject *window, const WINDOWDATA *data);
    // You can modify the given window's data directly, it's the same with using
    // setWindowData.
    static WINDOWDATA *windowData(void *window /* HWND */);
    static WINDOWDATA *windowData(QObject *window);

    // Change settings globally, not a specific window.
    // These values will be scaled automatically according to DPI, don't scale
    // them yourself. Just pass the original value.
    static void setBorderWidth(const int bw);
    static void setBorderHeight(const int bh);
    static void setTitleBarHeight(const int tbh);

    // System metric value of the given window (if the pointer is null,
    // return the system's standard value).
    static int getSystemMetric(void *handle /* HWND */,
                               const SystemMetric metric,
                               const bool dpiAware = false);

    // Use this function to trigger a frame change event or redraw a
    // specific window. Useful when you want to let some changes
    // in effect immediately.
    static void updateWindow(void *handle /* HWND */,
                             const bool triggerFrameChange = true,
                             const bool redraw = true);

    // Change the geometry of a window through Win32 API.
    // The width and height will be scaled automatically according to DPI. So
    // just pass the original value.
    static void setWindowGeometry(
        void *handle /* HWND */, const int x, const int y, const int width, const int height);

    // Move the window to the center of the desktop.
    static void moveWindowToDesktopCenter(void *handle /* HWND */);

    // Update Qt's internal data about the window frame, otherwise Qt will
    // take the size of the window frame into account when anyone is trying to
    // change the geometry of the window. That's not what we want.
    static void updateQtFrame(QWindow *window, const int titleBarHeight);

    // Display the system context menu.
    static bool displaySystemMenu(void *handle /* HWND */,
                                  const bool isRtl,
                                  const int x,
                                  const int y);

    // Enable or disable the blur effect for a specific window.
    // On Win10 it's the Acrylic effect.
    static bool setBlurEffectEnabled(void *handle /* HWND */,
                                     const bool enabled = true,
                                     const QColor &gradientColor = Qt::white);

    // Thin wrapper of DwmExtendFrameIntoClientArea().
    static void updateFrameMargins(void *handle /* HWND */);

    // A resizable window can be resized and maximized, however, a fixed size
    // window can only be moved and minimized, it can't be resized and maximized.
    static void setWindowResizable(void *handle /* HWND */, const bool resizable = true);

    // Query whether colorization is enabled or not.
    static bool colorizationEnabled();

    // Acquire the theme/colorization color set by the user.
    static QColor colorizationColor();

    // Query whether the user is using the light theme or not.
    static bool lightThemeEnabled();

    // Query whether the user is using the dark theme or not.
    static bool darkThemeEnabled();

    // Query whether the high contrast mode is enabled or not.
    static bool highContrastModeEnabled();

    // Query whether the given window is using dark frame or not.
    static bool darkFrameEnabled(void *handle /* HWND */);

    // Query whether the transparency effect is enabled or not.
    static bool transparencyEffectEnabled();

    ///////////////////////////////////////////////
    ///   CORE FUNCTION - THE SOUL OF THIS CODE
    ///////////////////////////////////////////////
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
#endif
};
