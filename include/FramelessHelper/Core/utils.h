/*
 * MIT License
 *
 * Copyright (C) 2021-2023 by wangwenx190 (Yuhang Zhao)
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

#include <FramelessHelper/Core/framelesshelpercore_global.h>
#ifdef Q_OS_LINUX
#  include <FramelessHelper/Core/framelesshelper_linux.h>
#endif // Q_OS_LINUX

#ifdef Q_OS_LINUX
QT_BEGIN_NAMESPACE
class QScreen;
QT_END_NAMESPACE
#endif // Q_OS_LINUX

FRAMELESSHELPER_BEGIN_NAMESPACE

struct SystemParameters;

namespace Utils
{

[[nodiscard]] FRAMELESSHELPER_CORE_API
    Qt::CursorShape calculateCursorShape(const QWindow *window, const QPoint &pos);
[[nodiscard]] FRAMELESSHELPER_CORE_API
    Qt::Edges calculateWindowEdges(const QWindow *window, const QPoint &pos);
FRAMELESSHELPER_CORE_API void startSystemMove(QWindow *window, const QPoint &globalPos);
FRAMELESSHELPER_CORE_API void startSystemResize(QWindow *window, const Qt::Edges edges, const QPoint &globalPos);
[[nodiscard]] FRAMELESSHELPER_CORE_API QString getSystemButtonGlyph(const Global::SystemButtonType button);
[[nodiscard]] FRAMELESSHELPER_CORE_API QWindow *findWindow(const WId windowId);
FRAMELESSHELPER_CORE_API void moveWindowToDesktopCenter(
    const SystemParameters *params, const bool considerTaskBar);
[[nodiscard]] FRAMELESSHELPER_CORE_API Qt::WindowState windowStatesToWindowState(
    const Qt::WindowStates states);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isThemeChangeEvent(const QEvent * const event);
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor calculateSystemButtonBackgroundColor(
    const Global::SystemButtonType button, const Global::ButtonState state);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool shouldAppsUseDarkMode();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isTitleBarColorized();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool
    setBlurBehindWindowEnabled(const WId windowId, const Global::BlurMode mode, const QColor &color);
[[nodiscard]] FRAMELESSHELPER_CORE_API QString getWallpaperFilePath();
[[nodiscard]] FRAMELESSHELPER_CORE_API Global::WallpaperAspectStyle getWallpaperAspectStyle();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isBlurBehindWindowSupported();
FRAMELESSHELPER_CORE_API void registerThemeChangeNotification();
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor getFrameBorderColor(const bool active);
[[nodiscard]] FRAMELESSHELPER_CORE_API qreal roundScaleFactor(const qreal factor);
[[nodiscard]] FRAMELESSHELPER_CORE_API int toNativePixels(const QWindow *window, const int value);
[[nodiscard]] FRAMELESSHELPER_CORE_API QPoint toNativePixels(const QWindow *window, const QPoint &point);
[[nodiscard]] FRAMELESSHELPER_CORE_API QSize toNativePixels(const QWindow *window, const QSize &size);
[[nodiscard]] FRAMELESSHELPER_CORE_API QRect toNativePixels(const QWindow *window, const QRect &rect);
[[nodiscard]] FRAMELESSHELPER_CORE_API int fromNativePixels(const QWindow *window, const int value);
[[nodiscard]] FRAMELESSHELPER_CORE_API QPoint fromNativePixels(const QWindow *window, const QPoint &point);
[[nodiscard]] FRAMELESSHELPER_CORE_API QSize fromNativePixels(const QWindow *window, const QSize &size);
[[nodiscard]] FRAMELESSHELPER_CORE_API QRect fromNativePixels(const QWindow *window, const QRect &rect);
[[nodiscard]] FRAMELESSHELPER_CORE_API QPoint toNativeLocalPosition(const QWindow *window, const QPoint &point);
[[nodiscard]] FRAMELESSHELPER_CORE_API QPoint toNativeGlobalPosition(const QWindow *window, const QPoint &point);
[[nodiscard]] FRAMELESSHELPER_CORE_API QPoint fromNativeLocalPosition(const QWindow *window, const QPoint &point);
[[nodiscard]] FRAMELESSHELPER_CORE_API QPoint fromNativeGlobalPosition(const QWindow *window, const QPoint &point);
[[nodiscard]] FRAMELESSHELPER_CORE_API int horizontalAdvance(const QFontMetrics &fm, const QString &str);
[[nodiscard]] FRAMELESSHELPER_CORE_API qreal getRelativeScaleFactor(const quint32 oldDpi, const quint32 newDpi);
[[nodiscard]] FRAMELESSHELPER_CORE_API QSizeF rescaleSize(const QSizeF &oldSize, const quint32 oldDpi, const quint32 newDpi);
[[nodiscard]] FRAMELESSHELPER_CORE_API QSize rescaleSize(const QSize &oldSize, const quint32 oldDpi, const quint32 newDpi);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isValidGeometry(const QRectF &rect);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isValidGeometry(const QRect &rect);
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor getAccentColor();
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 defaultScreenDpi();

#ifdef Q_OS_WINDOWS
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWindowsVersionOrGreater(const Global::WindowsVersion version);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isDwmCompositionEnabled();
FRAMELESSHELPER_CORE_API void triggerFrameChange(const WId windowId);
FRAMELESSHELPER_CORE_API void updateWindowFrameMargins(const WId windowId, const bool reset);
FRAMELESSHELPER_CORE_API void updateInternalWindowFrameMargins(QWindow *window, const bool enable);
[[nodiscard]] FRAMELESSHELPER_CORE_API QString getSystemErrorMessage(const QString &function);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isFullScreen(const WId windowId);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWindowNoState(const WId windowId);
FRAMELESSHELPER_CORE_API void syncWmPaintWithDwm();
FRAMELESSHELPER_CORE_API void showSystemMenu(
    const WId windowId, const QPoint &pos,
    const bool selectFirstEntry, const SystemParameters *params);
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor getDwmColorizationColor();
[[nodiscard]] FRAMELESSHELPER_CORE_API Global::DwmColorizationArea getDwmColorizationArea();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isHighContrastModeEnabled();
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getPrimaryScreenDpi(const bool horizontal);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getWindowDpi(const WId windowId, const bool horizontal);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getResizeBorderThicknessForDpi
    (const bool horizontal, const quint32 dpi);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getResizeBorderThickness(const WId windowId,
                                                                        const bool horizontal,
                                                                        const bool scaled);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getCaptionBarHeightForDpi(const quint32 dpi);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getCaptionBarHeight(const WId windowId, const bool scaled);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getTitleBarHeightForDpi(const quint32 dpi);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getTitleBarHeight(const WId windowId, const bool scaled);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getFrameBorderThicknessForDpi(const quint32 dpi);
[[nodiscard]] FRAMELESSHELPER_CORE_API quint32 getFrameBorderThickness(const WId windowId,
                                                                       const bool scaled);
FRAMELESSHELPER_CORE_API void maybeFixupQtInternals(const WId windowId);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isWindowFrameBorderVisible();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isFrameBorderColorized();
FRAMELESSHELPER_CORE_API void installSystemMenuHook(
    const WId windowId, const SystemParameters *params);
FRAMELESSHELPER_CORE_API void uninstallSystemMenuHook(const WId windowId);
FRAMELESSHELPER_CORE_API void setAeroSnappingEnabled(const WId windowId, const bool enable);
FRAMELESSHELPER_CORE_API void tryToEnableHighestDpiAwarenessLevel();
FRAMELESSHELPER_CORE_API void updateGlobalWin32ControlsTheme(const WId windowId, const bool dark);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool shouldAppsUseDarkMode_windows();
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor getAccentColor_windows();
FRAMELESSHELPER_CORE_API void setCornerStyleForWindow(const WId windowId, const Global::WindowCornerStyle style);
FRAMELESSHELPER_CORE_API void hideOriginalTitleBarElements
    (const WId windowId, const bool disable = true);
FRAMELESSHELPER_CORE_API void setQtDarkModeAwareEnabled(const bool enable);
FRAMELESSHELPER_CORE_API void refreshWin32ThemeResources(const WId windowId, const bool dark);
FRAMELESSHELPER_CORE_API void enableNonClientAreaDpiScalingForWindow(const WId windowId);
[[nodiscard]] FRAMELESSHELPER_CORE_API
    Global::DpiAwareness getDpiAwarenessForCurrentProcess(bool *highest = nullptr);
FRAMELESSHELPER_CORE_API void fixupChildWindowsDpiMessage(const WId windowId);
FRAMELESSHELPER_CORE_API void fixupDialogsDpiScaling();
FRAMELESSHELPER_CORE_API void setDarkModeAllowedForApp(const bool allow = true);
FRAMELESSHELPER_CORE_API void bringWindowToFront(const WId windowId);
[[nodiscard]] FRAMELESSHELPER_CORE_API QPoint getWindowPlacementOffset(const WId windowId);
[[nodiscard]] FRAMELESSHELPER_CORE_API QRect getWindowRestoreGeometry(const WId windowId);
FRAMELESSHELPER_CORE_API void removeMicaWindow(const WId windowId);
FRAMELESSHELPER_CORE_API void removeSysMenuHook(const WId windowId);
FRAMELESSHELPER_CORE_API quint64 queryMouseButtonState();
FRAMELESSHELPER_CORE_API bool isValidWindow(const WId windowId, const bool checkVisible, const bool checkTopLevel);
#endif // Q_OS_WINDOWS

#ifdef Q_OS_LINUX
[[nodiscard]] FRAMELESSHELPER_CORE_API QScreen *x11_findScreenForVirtualDesktop
    (const int virtualDesktopNumber);
[[nodiscard]] FRAMELESSHELPER_CORE_API x11_return_type x11_appRootWindow(const int screen);
[[nodiscard]] FRAMELESSHELPER_CORE_API int x11_appScreen();
[[nodiscard]] FRAMELESSHELPER_CORE_API x11_return_type x11_appTime();
[[nodiscard]] FRAMELESSHELPER_CORE_API x11_return_type x11_appUserTime();
[[nodiscard]] FRAMELESSHELPER_CORE_API x11_return_type x11_getTimestamp();
[[nodiscard]] FRAMELESSHELPER_CORE_API QByteArray x11_nextStartupId();
[[nodiscard]] FRAMELESSHELPER_CORE_API Display *x11_display();
[[nodiscard]] FRAMELESSHELPER_CORE_API xcb_connection_t *x11_connection();
[[nodiscard]] FRAMELESSHELPER_CORE_API QByteArray getWindowProperty
    (const WId windowId, const xcb_atom_t prop, const xcb_atom_t type, const quint32 data_len);
FRAMELESSHELPER_CORE_API void setWindowProperty
    (const WId windowId, const xcb_atom_t prop, const xcb_atom_t type,
     const void *data, const quint32 data_len, const uint8_t format);
FRAMELESSHELPER_CORE_API void clearWindowProperty(const WId windowId, const xcb_atom_t prop);
[[nodiscard]] FRAMELESSHELPER_CORE_API xcb_atom_t internAtom(const char *name);
[[nodiscard]] FRAMELESSHELPER_CORE_API QString getWindowManagerName();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isSupportedByWindowManager(const xcb_atom_t atom);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isSupportedByRootWindow(const xcb_atom_t atom);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool tryHideSystemTitleBar(const WId windowId, const bool hide = true);
FRAMELESSHELPER_CORE_API void openSystemMenu(const WId windowId, const QPoint &globalPos);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool shouldAppsUseDarkMode_linux();
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor getAccentColor_linux();
FRAMELESSHELPER_CORE_API void sendMoveResizeMessage
    (const WId windowId, const uint32_t action, const QPoint &globalPos, const Qt::MouseButton button = Qt::LeftButton);
[[nodiscard]] FRAMELESSHELPER_CORE_API bool isCustomDecorationSupported();
[[nodiscard]] FRAMELESSHELPER_CORE_API bool
    setPlatformPropertiesForWindow(QWindow *window, const QVariantHash &props);
#endif // Q_OS_LINUX

#ifdef Q_OS_MACOS
[[nodiscard]] FRAMELESSHELPER_CORE_API bool shouldAppsUseDarkMode_macos();
[[nodiscard]] FRAMELESSHELPER_CORE_API QColor getAccentColor_macos();
FRAMELESSHELPER_CORE_API void setSystemTitleBarVisible(const WId windowId, const bool visible);
FRAMELESSHELPER_CORE_API void removeWindowProxy(const WId windowId);
#endif // Q_OS_MACOS
} // namespace Utils

FRAMELESSHELPER_END_NAMESPACE
