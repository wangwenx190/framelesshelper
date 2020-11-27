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
#include <QObject>

QT_BEGIN_NAMESPACE
QT_FORWARD_DECLARE_CLASS(QWindow)
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
    enum class SystemMetric { BorderWidth, BorderHeight, TitleBarHeight };

    explicit WinNativeEventFilter();
    ~WinNativeEventFilter() override;

    static void addFramelessWindow(QWindow *window);
    static bool isWindowFrameless(const QWindow *window);
    static void removeFramelessWindow(QWindow *window);

    static void setIgnoredObjects(QWindow *window, const QObjectList &objects);
    static QObjectList getIgnoredObjects(const QWindow *window);

    static void setBorderWidth(QWindow *window, const int bw);
    static void setBorderHeight(QWindow *window, const int bh);
    static void setTitleBarHeight(QWindow *window, const int tbh);

    static int getSystemMetric(const QWindow *window,
                               const SystemMetric metric,
                               const bool dpiAware,
                               const bool forceSystemValue = false);

    // Enable or disable the blur effect for a specific window.
    // On Win10 it's the Acrylic effect.
    static bool setBlurEffectEnabled(const QWindow *window,
                                     const bool enabled,
                                     const QColor &gradientColor = Qt::white);

    // Query whether colorization is enabled or not.
    static bool isColorizationEnabled();

    // Acquire the theme/colorization color set by the user.
    static QColor getColorizationColor();

    // Query whether the user is using the light theme or not.
    static bool isLightThemeEnabled();

    // Query whether the user is using the dark theme or not.
    static bool isDarkThemeEnabled();

    // Query whether the high contrast mode is enabled or not.
    static bool isHighContrastModeEnabled();

    // Query whether the given window is using dark frame or not.
    static bool isDarkFrameEnabled(const QWindow *window);

    // Query whether the transparency effect is enabled or not.
    static bool isTransparencyEffectEnabled();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
#endif
};
