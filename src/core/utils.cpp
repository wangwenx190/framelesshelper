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

#include "utils.h"
#include "framelesshelpercore_global_p.h"
#ifdef Q_OS_WINDOWS
#  include "winverhelper_p.h"
#endif // Q_OS_WINDOWS
#include <array>
#include <QtCore/qloggingcategory.h>
#include <QtGui/qwindow.h>
#include <QtGui/qscreen.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qfontmetrics.h>
#include <QtGui/qpalette.h>
#include <QtGui/qsurface.h>
#include <QtGui/qsurfaceformat.h>
#include <QtGui/qevent.h>
#ifndef FRAMELESSHELPER_CORE_NO_PRIVATE
#  include <QtGui/private/qhighdpiscaling_p.h>
#  include <QtGui/private/qwindow_p.h>
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
#  include <QtGui/qstylehints.h>
#elif ((QT_VERSION >= QT_VERSION_CHECK(6, 2, 1)) && !defined(FRAMELESSHELPER_CORE_NO_PRIVATE))
#  include <QtGui/qpa/qplatformtheme.h>
#  include <QtGui/private/qguiapplication_p.h>
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))

FRAMELESSHELPER_BEGIN_NAMESPACE

[[maybe_unused]] static Q_LOGGING_CATEGORY(lcUtilsCommon, "wangwenx190.framelesshelper.core.utils.common")

#ifdef FRAMELESSHELPER_CORE_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcUtilsCommon)
#  define DEBUG qCDebug(lcUtilsCommon)
#  define WARNING qCWarning(lcUtilsCommon)
#  define CRITICAL qCCritical(lcUtilsCommon)
#endif

using namespace Global;

#ifndef FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE
struct FONT_ICON
{
    quint32 SegoeUI = 0;
    quint32 Fallback = 0;
};

static constexpr const std::array<FONT_ICON, static_cast<int>(SystemButtonType::Last) + 1> g_fontIconsTable =
{
    FONT_ICON{ 0x0000, 0x0000 },
    FONT_ICON{ 0xE756, 0x0000 },
    FONT_ICON{ 0xE897, 0x0000 },
    FONT_ICON{ 0xE921, 0xE93E },
    FONT_ICON{ 0xE922, 0xE93C },
    FONT_ICON{ 0xE923, 0xE93D },
    FONT_ICON{ 0xE8BB, 0xE93B }
};
#endif // FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE

#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
[[nodiscard]] static inline QPoint getScaleOrigin(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
    QScreen *screen = window->screen();
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return {};
    }
    return screen->geometry().topLeft();
}
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE

Qt::CursorShape Utils::calculateCursorShape(const QWindow *window, const QPoint &pos)
{
#ifdef Q_OS_MACOS
    Q_UNUSED(window);
    Q_UNUSED(pos);
    return Qt::ArrowCursor;
#else
    Q_ASSERT(window);
    if (!window) {
        return Qt::ArrowCursor;
    }
    if (window->visibility() != QWindow::Windowed) {
        return Qt::ArrowCursor;
    }
    const int x = pos.x();
    const int y = pos.y();
    const int w = window->width();
    const int h = window->height();
    if (((x < kDefaultResizeBorderThickness) && (y < kDefaultResizeBorderThickness))
        || ((x >= (w - kDefaultResizeBorderThickness)) && (y >= (h - kDefaultResizeBorderThickness)))) {
        return Qt::SizeFDiagCursor;
    }
    if (((x >= (w - kDefaultResizeBorderThickness)) && (y < kDefaultResizeBorderThickness))
        || ((x < kDefaultResizeBorderThickness) && (y >= (h - kDefaultResizeBorderThickness)))) {
        return Qt::SizeBDiagCursor;
    }
    if ((x < kDefaultResizeBorderThickness) || (x >= (w - kDefaultResizeBorderThickness))) {
        return Qt::SizeHorCursor;
    }
    if ((y < kDefaultResizeBorderThickness) || (y >= (h - kDefaultResizeBorderThickness))) {
        return Qt::SizeVerCursor;
    }
    return Qt::ArrowCursor;
#endif
}

Qt::Edges Utils::calculateWindowEdges(const QWindow *window, const QPoint &pos)
{
#ifdef Q_OS_MACOS
    Q_UNUSED(window);
    Q_UNUSED(pos);
    return {};
#else
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
    if (window->visibility() != QWindow::Windowed) {
        return {};
    }
    Qt::Edges edges = {};
    const int x = pos.x();
    const int y = pos.y();
    if (x < kDefaultResizeBorderThickness) {
        edges |= Qt::LeftEdge;
    }
    if (x >= (window->width() - kDefaultResizeBorderThickness)) {
        edges |= Qt::RightEdge;
    }
    if (y < kDefaultResizeBorderThickness) {
        edges |= Qt::TopEdge;
    }
    if (y >= (window->height() - kDefaultResizeBorderThickness)) {
        edges |= Qt::BottomEdge;
    }
    return edges;
#endif
}

QString Utils::getSystemButtonGlyph(const SystemButtonType button)
{
#ifdef FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE
    return {};
#else // !FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE
    const FONT_ICON &icon = g_fontIconsTable.at(static_cast<int>(button));
#  ifdef Q_OS_WINDOWS
    // Windows 11: Segoe Fluent Icons (https://docs.microsoft.com/en-us/windows/apps/design/style/segoe-fluent-icons-font)
    // Windows 10: Segoe MDL2 Assets (https://docs.microsoft.com/en-us/windows/apps/design/style/segoe-ui-symbol-font)
    // Windows 7~8.1: Our own custom icon
    if (WindowsVersionHelper::isWin10OrGreater()) {
        return QChar(icon.SegoeUI);
    }
#  endif // Q_OS_WINDOWS
    // We always use our own icons on UNIX platforms because Microsoft doesn't allow distributing
    // the Segoe icon font to other platforms than Windows.
    return QChar(icon.Fallback);
#endif // FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE
}

QWindow *Utils::findWindow(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return nullptr;
    }
    const QWindowList windows = QGuiApplication::topLevelWindows();
    if (windows.isEmpty()) {
        return nullptr;
    }
    for (auto &&window : std::as_const(windows)) {
        if (window && window->handle()) {
            if (window->winId() == windowId) {
                return window;
            }
        }
    }
    return nullptr;
}

void Utils::moveWindowToDesktopCenter(FramelessParamsConst params, const bool considerTaskBar)
{
    Q_ASSERT(params);
    if (!params) {
        return;
    }
    const QSize windowSize = params->getWindowSize();
    if (windowSize.isEmpty() || (windowSize == kDefaultWindowSize)) {
        return;
    }
    const QScreen *screen = params->getWindowScreen();
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    Q_ASSERT(screen);
    if (!screen) {
        return;
    }
    const QSize screenSize = (considerTaskBar ? screen->availableSize() : screen->size());
    const QPoint offset = (considerTaskBar ? screen->availableGeometry().topLeft() : QPoint(0, 0));
    const int newX = std::round(qreal(screenSize.width() - windowSize.width()) / qreal(2));
    const int newY = std::round(qreal(screenSize.height() - windowSize.height()) / qreal(2));
    params->setWindowPosition(QPoint(newX + offset.x(), newY + offset.y()));
}

Qt::WindowState Utils::windowStatesToWindowState(const Qt::WindowStates states)
{
    if (states & Qt::WindowFullScreen) {
        return Qt::WindowFullScreen;
    }
    if (states & Qt::WindowMaximized) {
        return Qt::WindowMaximized;
    }
    if (states & Qt::WindowMinimized) {
        return Qt::WindowMinimized;
    }
    return Qt::WindowNoState;
}

bool Utils::isThemeChangeEvent(const QEvent * const event)
{
    Q_ASSERT(event);
    if (!event) {
        return false;
    }
    // QGuiApplication will only deliver theme change events to top level Q(Quick)Windows,
    // QWidgets won't get such notifications, no matter whether it's top level widget or not.
    // QEvent::ThemeChange: Send by the Windows QPA.
    // QEvent::ApplicationPaletteChange: All other platforms (Linux & macOS).
    const QEvent::Type type = event->type();
    return ((type == QEvent::ThemeChange) || (type == QEvent::ApplicationPaletteChange));
}

QColor Utils::calculateSystemButtonBackgroundColor(const SystemButtonType button, const ButtonState state)
{
    if (state == ButtonState::Normal) {
        return kDefaultTransparentColor;
    }
    const bool isClose = (button == SystemButtonType::Close);
    const bool isTitleColor = isTitleBarColorized();
    const bool isHovered = (state == ButtonState::Hovered);
    const auto result = [isClose, isTitleColor]() -> QColor {
        if (isClose) {
            return kDefaultSystemCloseButtonBackgroundColor;
        }
        if (isTitleColor) {
            return getAccentColor();
        }
        return kDefaultSystemButtonBackgroundColor;
    }();
    if (isClose) {
        return (isHovered ? result.lighter(110) : result.lighter(140));
    }
    if (!isTitleColor) {
        return (isHovered ? result.lighter(110) : result);
    }
    return (isHovered ? result.lighter(150) : result.lighter(120));
}

bool Utils::shouldAppsUseDarkMode()
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
    return (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark);
#elif ((QT_VERSION >= QT_VERSION_CHECK(6, 2, 1)) && !defined(FRAMELESSHELPER_CORE_NO_PRIVATE))
    if (const QPlatformTheme * const theme = QGuiApplicationPrivate::platformTheme()) {
        return (theme->appearance() == QPlatformTheme::Appearance::Dark);
    }
    return false;
#else // ((QT_VERSION < QT_VERSION_CHECK(6, 2, 1)) || FRAMELESSHELPER_CORE_NO_PRIVATE)
#  ifdef Q_OS_WINDOWS
    return shouldAppsUseDarkMode_windows();
#  elif defined(Q_OS_LINUX)
    return shouldAppsUseDarkMode_linux();
#  elif defined(Q_OS_MACOS)
    return shouldAppsUseDarkMode_macos();
#  else
    return false;
#  endif
#endif
}

qreal Utils::roundScaleFactor(const qreal factor)
{
    // Qt can't handle scale factors less than 1.0 (according to the comments in qhighdpiscaling.cpp).
    Q_ASSERT((factor > 1) || qFuzzyCompare(factor, qreal(1)));
    if (factor < 1) {
        return 1;
    }
#if (defined(FRAMELESSHELPER_CORE_NO_PRIVATE) || (QT_VERSION < QT_VERSION_CHECK(6, 2, 1)))
#  if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    static const auto policy = QGuiApplication::highDpiScaleFactorRoundingPolicy();
    switch (policy) {
    case Qt::HighDpiScaleFactorRoundingPolicy::Round:
        return std::round(factor);
    case Qt::HighDpiScaleFactorRoundingPolicy::Ceil:
        return std::ceil(factor);
    case Qt::HighDpiScaleFactorRoundingPolicy::Floor:
        return std::floor(factor);
    case Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor:
    {
        static constexpr const auto flag = qreal(0.75);
        const qreal gap = (factor - qreal(int(factor)));
        return (((gap > flag) || qFuzzyCompare(gap, flag)) ? std::round(factor) : std::floor(factor));
    }
    case Qt::HighDpiScaleFactorRoundingPolicy::PassThrough:
    case Qt::HighDpiScaleFactorRoundingPolicy::Unset: // According to Qt source code, this enum value is the same with PassThrough.
        return factor;
    }
    return 1;
#  else // (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    return std::round(factor);
#  endif // (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#else // (!FRAMELESSHELPER_CORE_NO_PRIVATE && (QT_VERSION >= QT_VERSION_CHECK(6, 2, 1)))
    return QHighDpiScaling::roundScaleFactor(factor);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

int Utils::toNativePixels(const QWindow *window, const int value)
{
    Q_ASSERT(window);
    if (!window) {
        return 0;
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    return std::round(qreal(value) * window->devicePixelRatio());
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::toNativePixels(value, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

QPoint Utils::toNativePixels(const QWindow *window, const QPoint &point)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    const QPoint origin = getScaleOrigin(window);
    return QPointF(QPointF(point - origin) * window->devicePixelRatio()).toPoint() + origin;
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::toNativePixels(point, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

QSize Utils::toNativePixels(const QWindow *window, const QSize &size)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    return QSizeF(QSizeF(size) * window->devicePixelRatio()).toSize();
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::toNativePixels(size, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

QRect Utils::toNativePixels(const QWindow *window, const QRect &rect)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    return QRect(toNativePixels(window, rect.topLeft()), toNativePixels(window, rect.size()));
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::toNativePixels(rect, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

int Utils::fromNativePixels(const QWindow *window, const int value)
{
    Q_ASSERT(window);
    if (!window) {
        return 0;
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    return std::round(qreal(value) / window->devicePixelRatio());
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::fromNativePixels(value, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

QPoint Utils::fromNativePixels(const QWindow *window, const QPoint &point)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    const QPoint origin = getScaleOrigin(window);
    return QPointF(QPointF(point - origin) / window->devicePixelRatio()).toPoint() + origin;
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::fromNativePixels(point, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

QSize Utils::fromNativePixels(const QWindow *window, const QSize &size)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    return QSizeF(QSizeF(size) / window->devicePixelRatio()).toSize();
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::fromNativePixels(size, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

QRect Utils::fromNativePixels(const QWindow *window, const QRect &rect)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    return QRect(fromNativePixels(window, rect.topLeft()), fromNativePixels(window, rect.size()));
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::fromNativePixels(rect, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

QPoint Utils::toNativeLocalPosition(const QWindow *window, const QPoint &point)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    return QPointF(QPointF(point) * window->devicePixelRatio()).toPoint();
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::toNativeLocalPosition(point, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

QPoint Utils::toNativeGlobalPosition(const QWindow *window, const QPoint &point)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
#if (defined(FRAMELESSHELPER_CORE_NO_PRIVATE) || (QT_VERSION < QT_VERSION_CHECK(6, 0, 0)))
    return toNativePixels(window, point);
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE && QT_VERSION >= 6.0.0
    return QHighDpi::toNativeGlobalPosition(point, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE || QT_VERSION < 6.0.0
}

QPoint Utils::fromNativeLocalPosition(const QWindow *window, const QPoint &point)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    return QPointF(QPointF(point) / window->devicePixelRatio()).toPoint();
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::fromNativeLocalPosition(point, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

QPoint Utils::fromNativeGlobalPosition(const QWindow *window, const QPoint &point)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
#if (defined(FRAMELESSHELPER_CORE_NO_PRIVATE) || (QT_VERSION < QT_VERSION_CHECK(6, 0, 0)))
    return fromNativePixels(window, point);
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE && QT_VERSION >= 6.0.0
    return QHighDpi::fromNativeGlobalPosition(point, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE || QT_VERSION < 6.0.0
}

int Utils::horizontalAdvance(const QFontMetrics &fm, const QString &str)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    return fm.horizontalAdvance(str);
#else // (QT_VERSION < QT_VERSION_CHECK(5, 11, 0))
    return fm.width();
#endif // (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
}

qreal Utils::getRelativeScaleFactor(const quint32 oldDpi, const quint32 newDpi)
{
    if (newDpi == oldDpi) {
        return qreal(1);
    }
    static const quint32 defaultDpi = defaultScreenDpi();
    if ((oldDpi < defaultDpi) || (newDpi < defaultDpi)) {
        return qreal(1);
    }
    // We need to round the scale factor according to Qt's rounding policy.
    const qreal oldDpr = roundScaleFactor(qreal(oldDpi) / qreal(defaultDpi));
    const qreal newDpr = roundScaleFactor(qreal(newDpi) / qreal(defaultDpi));
    return qreal(newDpr / oldDpr);
}

QSizeF Utils::rescaleSize(const QSizeF &oldSize, const quint32 oldDpi, const quint32 newDpi)
{
    if (oldSize.isEmpty()) {
        return {};
    }
    if (newDpi == oldDpi) {
        return oldSize;
    }
    const qreal scaleFactor = getRelativeScaleFactor(oldDpi, newDpi);
    if (qFuzzyIsNull(scaleFactor)) {
        return {};
    }
    if (qFuzzyCompare(scaleFactor, qreal(1))) {
        return oldSize;
    }
    return QSizeF(oldSize * scaleFactor);
}

QSize Utils::rescaleSize(const QSize &oldSize, const quint32 oldDpi, const quint32 newDpi)
{
    return rescaleSize(QSizeF(oldSize), oldDpi, newDpi).toSize();
}

bool Utils::isValidGeometry(const QRectF &rect)
{
    // The position of the rectangle is not relevant.
    return ((rect.right() > rect.left()) && (rect.bottom() > rect.top()));
}

bool Utils::isValidGeometry(const QRect &rect)
{
    return isValidGeometry(QRectF(rect));
}

quint32 Utils::defaultScreenDpi()
{
#ifdef Q_OS_MACOS
    return 72;
#else // !Q_OS_MACOS
    return 96;
#endif // Q_OS_MACOS
}

QColor Utils::getAccentColor()
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))
    return QGuiApplication::palette().color(QPalette::AccentColor);
#else // (QT_VERSION < QT_VERSION_CHECK(6, 6, 0))
#  ifdef Q_OS_WINDOWS
    return getAccentColor_windows();
#  elif defined(Q_OS_LINUX)
    return getAccentColor_linux();
#  elif defined(Q_OS_MACOS)
    return getAccentColor_macos();
#  else
    return QGuiApplication::palette().color(QPalette::Highlight);
#  endif
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))
}

bool Utils::isWindowAccelerated(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
    switch (window->surfaceType()) {
    case QSurface::RasterGLSurface:
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
        return true;
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
        return qt_window_private(const_cast<QWindow *>(window))->compositing;
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
    case QSurface::OpenGLSurface:
    case QSurface::VulkanSurface:
    case QSurface::MetalSurface:
#if (QT_VERSION >= QT_VERSION_CHECK(6, 1, 0))
    case QSurface::Direct3DSurface:
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 1, 0))
        return true;
    default:
        break;
    }
    return false;
}

bool Utils::isWindowTransparent(const QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
    // On most platforms, QWindow::format() will just return the
    // user set format if there is one, otherwise it will return
    // an invalid surface format. That means, most of the time
    // the following check will not be useful. But since this is
    // what the QPA code does, we just mirror it here.
    return window->format().hasAlpha();
}

void Utils::emulateQtMouseEvent(const QObject *target, const QWindow *window, const ButtonState buttonState,
    const QPoint &globalPos, const QPoint &scenePos, const QPoint &localPos, const bool underMouse, const bool enableHover)
{
    Q_ASSERT(target);
    Q_ASSERT(window);
    if (!target || !window) {
        return;
    }
    const auto targetObj = const_cast<QObject *>(target);
    const auto windowObj = static_cast<QObject *>(const_cast<QWindow *>(window));
    const bool isWidget = target->isWidgetType();
    static constexpr const char kMouseTrackingProp[] = "mouseTracking";
    const bool mouseTrackingEnabled = (isWidget ? target->property(kMouseTrackingProp).toBool() : true);
    const bool hoverEnabled = (isWidget ? enableHover : true);
    static constexpr const QPoint oldPos = {}; // Not needed.
    static constexpr const Qt::MouseButton button = Qt::LeftButton;
    const Qt::MouseButtons buttons = QGuiApplication::mouseButtons();
    const Qt::KeyboardModifiers modifiers = QGuiApplication::keyboardModifiers();
    static constexpr const char kEnteredFlag[] = "__FRAMELESSHELPER_WIDGET_QUICKITEM_ENTERED";
    const bool entered = target->property(kEnteredFlag).toBool();
    const bool leftButtonPressed = (queryMouseButtons() & Qt::LeftButton);
    const auto sendEnterEvent = [&localPos, &scenePos, &globalPos, &modifiers, hoverEnabled](QObject *obj) -> void {
        Q_ASSERT(obj);
        if (!obj) {
            return;
        }
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        QEnterEvent enterEvent(localPos, scenePos, globalPos);
#else
        QEvent enterEvent(QEvent::Enter);
#endif
        QCoreApplication::sendEvent(obj, &enterEvent);
        if (hoverEnabled) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
            QHoverEvent hoverEnterEvent(QEvent::HoverEnter, scenePos, globalPos, oldPos, modifiers);
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
            QHoverEvent hoverEnterEvent(QEvent::HoverEnter, localPos, globalPos, oldPos, modifiers);
#else
            QHoverEvent hoverEnterEvent(QEvent::HoverEnter, localPos, oldPos, modifiers);
#endif
            QCoreApplication::sendEvent(obj, &hoverEnterEvent);
        }
    };
    const auto sendLeaveEvent = [&localPos, &scenePos, &globalPos, &modifiers, hoverEnabled](QObject *obj) -> void {
        Q_ASSERT(obj);
        if (!obj) {
            return;
        }
        QEvent leaveEvent(QEvent::Leave);
        QCoreApplication::sendEvent(obj, &leaveEvent);
        if (hoverEnabled) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
            QHoverEvent hoverLeaveEvent(QEvent::HoverLeave, scenePos, globalPos, oldPos, modifiers);
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
            QHoverEvent hoverLeaveEvent(QEvent::HoverLeave, localPos, globalPos, oldPos, modifiers);
#else
            QHoverEvent hoverLeaveEvent(QEvent::HoverLeave, localPos, oldPos, modifiers);
#endif
            QCoreApplication::sendEvent(obj, &hoverLeaveEvent);
        }
    };
    const auto sendMouseMoveEvent = [&localPos, &scenePos, &globalPos, &buttons, &modifiers, leftButtonPressed](QObject *obj) -> void {
        Q_ASSERT(obj);
        if (!obj) {
            return;
        }
        const Qt::MouseButton actualButton = (leftButtonPressed ? button : Qt::NoButton);
        QMouseEvent event(QEvent::MouseMove, localPos, scenePos, globalPos, actualButton, buttons, modifiers);
        QCoreApplication::sendEvent(obj, &event);
    };
    const auto sendHoverMoveEvent = [&localPos, &scenePos, &globalPos, &modifiers, hoverEnabled](QObject *obj) -> void {
        Q_ASSERT(obj);
        if (!obj) {
            return;
        }
        if (!hoverEnabled) {
            return;
        }
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
        QHoverEvent event(QEvent::HoverMove, scenePos, globalPos, oldPos, modifiers);
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
        QHoverEvent event(QEvent::HoverMove, localPos, globalPos, oldPos, modifiers);
#else
        QHoverEvent event(QEvent::HoverMove, localPos, oldPos, modifiers);
#endif
        QCoreApplication::sendEvent(obj, &event);
    };
    const auto sendMousePressEvent = [&localPos, &scenePos, &globalPos, &buttons, &modifiers](QObject *obj) -> void {
        Q_ASSERT(obj);
        if (!obj) {
            return;
        }
        QMouseEvent event(QEvent::MouseButtonPress, localPos, scenePos, globalPos, button, buttons, modifiers);
        QCoreApplication::sendEvent(obj, &event);
    };
    const auto sendMouseReleaseEvent = [&localPos, &scenePos, &globalPos, &buttons, &modifiers](QObject *obj, const bool fake = false) -> void {
        Q_ASSERT(obj);
        if (!obj) {
            return;
        }
        static constexpr const auto fakePos = QPoint{ -999, -999 };
        const QPoint tweakedLocalPos = (fake ? fakePos : localPos);
        const QPoint tweakedScenePos = (fake ? fakePos : scenePos);
        const QPoint tweakedGlobalPos = (fake ? fakePos : globalPos);
        QMouseEvent event(QEvent::MouseButtonRelease, tweakedLocalPos, tweakedScenePos, tweakedGlobalPos, button, buttons, modifiers);
        QCoreApplication::sendEvent(obj, &event);
    };
    switch (buttonState) {
    case ButtonState::Normal: {
        targetObj->setProperty(kEnteredFlag, {});
        // Send an extra mouse release event to let the control dismiss it's hover state.
        sendMouseReleaseEvent(targetObj, true);
        if (isWidget) {
            sendLeaveEvent(targetObj);
        } else {
            sendLeaveEvent(windowObj);
        }
    } break;
    case ButtonState::Hovered: {
        if (!entered) {
            targetObj->setProperty(kEnteredFlag, true);
            if (isWidget) {
                sendEnterEvent(targetObj);
            } else {
                sendEnterEvent(windowObj);
            }
        }
        if (isWidget) {
            sendHoverMoveEvent(targetObj);
        } else {
            sendHoverMoveEvent(windowObj);
        }
        if (leftButtonPressed || mouseTrackingEnabled) {
            if (isWidget) {
                sendMouseMoveEvent(targetObj);
            } else {
                sendMouseMoveEvent(windowObj);
            }
        }
    } break;
    case ButtonState::Pressed:
        // Sending mouse event to the window has no effect.
        sendMousePressEvent(targetObj);
        break;
    case ButtonState::Released:
        // Sending mouse event to the window has no effect.
        sendMouseReleaseEvent(targetObj);
        break;
    }
}

FRAMELESSHELPER_END_NAMESPACE
