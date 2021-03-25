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

#pragma once

#include "framelesshelper_global.h"
#include <QtGui/qcolor.h>
#include <QtGui/qwindow.h>

namespace Utilities {

enum class SystemMetric
{
    BorderWidth,
    BorderHeight,
    TitleBarHeight
};

enum class DesktopWallpaperAspectStyle
{
    Central,
    Tiled,
    IgnoreRatioFit, // Stretch
    KeepRatioFit, // Fit
    KeepRatioByExpanding, // Fill
    Span
};

// Common
FRAMELESSHELPER_EXPORT bool shouldUseWallpaperBlur();
FRAMELESSHELPER_EXPORT bool shouldUseTraditionalBlur();
FRAMELESSHELPER_EXPORT bool setBlurEffectEnabled(const QWindow *window, const bool enabled, const QColor &gradientColor = {});

FRAMELESSHELPER_EXPORT int getSystemMetric(const QWindow *window, const SystemMetric metric, const bool dpiAware, const bool forceSystemValue = false);

FRAMELESSHELPER_EXPORT bool isLightThemeEnabled();
FRAMELESSHELPER_EXPORT bool isDarkThemeEnabled();

FRAMELESSHELPER_EXPORT QWindow *findWindow(const WId winId);

FRAMELESSHELPER_EXPORT QImage getDesktopWallpaperImage(const int screen = -1);
FRAMELESSHELPER_EXPORT QColor getDesktopBackgroundColor(const int screen = -1);
FRAMELESSHELPER_EXPORT DesktopWallpaperAspectStyle getDesktopWallpaperAspectStyle(const int screen = -1);

FRAMELESSHELPER_EXPORT QRect getScreenAvailableGeometry(const QWindow *window);
FRAMELESSHELPER_EXPORT QRect getScreenAvailableGeometry(const QPoint &pos);

FRAMELESSHELPER_EXPORT QRect getScreenGeometry(const QWindow *window);
FRAMELESSHELPER_EXPORT QRect getScreenGeometry(const QPoint &pos);

FRAMELESSHELPER_EXPORT QRect alignedRect(const Qt::LayoutDirection direction, const Qt::Alignment alignment, const QSize &size, const QRect &rectangle);

FRAMELESSHELPER_EXPORT void blurImage(QImage &blurImage, const qreal radius, const bool quality, const int transposed = 0);
FRAMELESSHELPER_EXPORT void blurImage(QPainter *painter, QImage &blurImage, const qreal radius, const bool quality, const bool alphaOnly, const int transposed = 0);

FRAMELESSHELPER_EXPORT bool disableExtraProcessingForBlur();
FRAMELESSHELPER_EXPORT bool forceEnableTraditionalBlur();
FRAMELESSHELPER_EXPORT bool forceDisableWallpaperBlur();
FRAMELESSHELPER_EXPORT bool shouldUseNativeTitleBar();

FRAMELESSHELPER_EXPORT bool isWindowFixedSize(const QWindow *window);

FRAMELESSHELPER_EXPORT bool isMouseInSpecificObjects(const QPointF &mousePos, const QObjectList &objects, const qreal dpr = 1.0);

#ifdef Q_OS_WINDOWS
// Windows specific
FRAMELESSHELPER_EXPORT bool isWin7OrGreater();
FRAMELESSHELPER_EXPORT bool isWin8OrGreater();
FRAMELESSHELPER_EXPORT bool isWin8Point1OrGreater();
FRAMELESSHELPER_EXPORT bool isWin10OrGreater();
FRAMELESSHELPER_EXPORT bool isWin10OrGreater(const int subVer);

FRAMELESSHELPER_EXPORT bool isDwmBlurAvailable();
FRAMELESSHELPER_EXPORT bool isOfficialMSWin10AcrylicBlurAvailable();

FRAMELESSHELPER_EXPORT bool isColorizationEnabled();
FRAMELESSHELPER_EXPORT QColor getColorizationColor();

FRAMELESSHELPER_EXPORT bool isHighContrastModeEnabled();
FRAMELESSHELPER_EXPORT bool isDarkFrameEnabled(const QWindow *window);
FRAMELESSHELPER_EXPORT bool isTransparencyEffectEnabled();

FRAMELESSHELPER_EXPORT void triggerFrameChange(const QWindow *window);
FRAMELESSHELPER_EXPORT void updateFrameMargins(const QWindow *window, const bool reset);
FRAMELESSHELPER_EXPORT void updateQtFrameMargins(QWindow *window, const bool enable);

FRAMELESSHELPER_EXPORT quint32 getWindowDpi(const QWindow *window);
FRAMELESSHELPER_EXPORT QMargins getWindowNativeFrameMargins(const QWindow *window);
FRAMELESSHELPER_EXPORT QColor getNativeWindowFrameColor(const bool isActive = true);

FRAMELESSHELPER_EXPORT void displaySystemMenu(const QWindow *window, const QPoint &pos = {});
#endif

}
