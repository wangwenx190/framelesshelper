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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <QAbstractNativeEventFilter>
#include <QRect>
#include <QVector>
#include <qt_windows.h>

class WinNativeEventFilter : public QAbstractNativeEventFilter {
    Q_DISABLE_COPY_MOVE(WinNativeEventFilter)

public:
    using WINDOWDATA = struct _WINDOWDATA {
        BOOL fixedSize = FALSE, mouseTransparent = FALSE;
        int borderWidth = -1, borderHeight = -1, titlebarHeight = -1;
        QVector<QRect> ignoreAreas, draggableAreas;
        QSize maximumSize = {-1, -1}, minimumSize = {-1, -1};
    };

    using WINDOW = struct _WINDOW {
        HWND hWnd = nullptr;
        BOOL initialized = FALSE;
        WINDOWDATA windowData;
    };

    enum class SystemMetric { BorderWidth, BorderHeight, TitleBarHeight };

    explicit WinNativeEventFilter();
    ~WinNativeEventFilter() override;

    // Frameless windows handle list
    static QVector<HWND> framelessWindows();
    static void setFramelessWindows(QVector<HWND> windows);
    // Make the given window become frameless.
    static void addFramelessWindow(HWND window,
                                   const WINDOWDATA *data = nullptr);
    static void removeFramelessWindow(HWND window);
    static void clearFramelessWindows();

    // Set borderWidth, borderHeight or titlebarHeight to a negative value to
    // restore default behavior.
    // Note that it can only affect one specific window.
    // If you want to change these values globally, use setBorderWidth instead.
    static void setWindowData(HWND window, const WINDOWDATA *data);
    // You can modify the given window's data directly, it's the same with using
    // setWindowData.
    static WINDOWDATA *windowData(HWND window);

    // Change settings globally, not a specific window.
    static void setBorderWidth(int bw);
    static void setBorderHeight(int bh);
    static void setTitlebarHeight(int tbh);

    // System metric value of the given window (if the pointer is null,
    // return the system's standard value).
    static int getSystemMetric(HWND handle, SystemMetric metric,
                               bool dpiAware = true);

    // Use this function to trigger a frame change event or redraw a
    // specific window. Useful when you want to let some changes
    // in effect immediately.
    static void updateWindow(HWND handle, bool triggerFrameChange = true,
                             bool redraw = true);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    bool nativeEventFilter(const QByteArray &eventType, void *message,
                           qintptr *result) override;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message,
                           long *result) override;
#endif

private:
    // Do not call these two functions directly, otherwise strange things
    // will happen.
    static void install();
    static void uninstall();
};
