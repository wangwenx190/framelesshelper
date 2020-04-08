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
        BOOL blurEnabled = FALSE;
        int borderWidth = -1, borderHeight = -1, titlebarHeight = -1;
        QVector<QRect> ignoreAreas, draggableAreas;
        QSize maximumSize = {-1, -1}, minimumSize = {-1, -1};
    };

    using WINDOW = struct _WINDOW {
        HWND hWnd = nullptr;
        BOOL dwmCompositionEnabled = FALSE, themeEnabled = FALSE,
             inited = FALSE;
        WINDOWDATA windowData;
    };

    explicit WinNativeEventFilter();
    ~WinNativeEventFilter() override;

    // Make all top level windows become frameless, unconditionally.
    static void install();
    // Make all top level windows back to normal.
    static void uninstall();

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

    // DPI-aware border width of the given window (if the pointer is null,
    // return the system's standard value).
    static int borderWidth(HWND handle);
    // DPI-aware border height of the given window (if the pointer is null,
    // return the system's standard value).
    static int borderHeight(HWND handle);
    // DPI-aware titlebar height (including the border height) of the given window (if the pointer is null,
    // return the system's standard value).
    static int titlebarHeight(HWND handle);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    bool nativeEventFilter(const QByteArray &eventType, void *message,
                           qintptr *result) override;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message,
                           long *result) override;
#endif

private:
    void init(WINDOW *data);
    void initWin32Api();
    static void createUserData(HWND handle, const WINDOWDATA *data = nullptr);
    void handleDwmCompositionChanged(WINDOW *data);
    void handleThemeChanged(WINDOW *data);
    void handleBlurForWindow(const WINDOW *data);
    static void refreshWindow(HWND handle);
    static qreal getPreferedNumber(qreal num);
    static UINT getDotsPerInchForWindow(HWND handle);
    static qreal getDevicePixelRatioForWindow(HWND handle);
    static int getSystemMetricsForWindow(HWND handle, int index);
};
