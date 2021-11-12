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
#include <QtGui/qwindow.h>
#include <QtCore/qsize.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

namespace Utilities
{

FRAMELESSHELPER_API int getSystemMetric(const QWindow *window, const SystemMetric metric, const bool dpiScale, const bool forceSystemValue = false);
FRAMELESSHELPER_API QWindow *findWindow(const WId winId);
FRAMELESSHELPER_API bool shouldUseNativeTitleBar();
FRAMELESSHELPER_API bool isWindowFixedSize(const QWindow *window);
FRAMELESSHELPER_API bool isHitTestVisibleInChrome(const QWindow *window);
FRAMELESSHELPER_API QPointF mapOriginPointToWindow(const QObject *object);
FRAMELESSHELPER_API QColor getColorizationColor();
FRAMELESSHELPER_API int getWindowVisibleFrameBorderThickness(const WId winId);
FRAMELESSHELPER_API bool shouldAppsUseDarkMode();
FRAMELESSHELPER_API ColorizationArea getColorizationArea();
FRAMELESSHELPER_API bool isThemeChanged(const void *data);
FRAMELESSHELPER_API bool isSystemMenuRequested(const void *data, QPointF *pos);
FRAMELESSHELPER_API bool showSystemMenu(const WId winId, const QPointF &pos);

#ifdef Q_OS_WINDOWS
FRAMELESSHELPER_API bool isWin8OrGreater();
FRAMELESSHELPER_API bool isWin8Point1OrGreater();
FRAMELESSHELPER_API bool isWin10OrGreater();
FRAMELESSHELPER_API bool isDwmCompositionAvailable();
FRAMELESSHELPER_API void triggerFrameChange(const WId winId);
FRAMELESSHELPER_API void updateFrameMargins(const WId winId, const bool reset);
FRAMELESSHELPER_API void updateQtFrameMargins(QWindow *window, const bool enable);
FRAMELESSHELPER_API QString getSystemErrorMessage(const QString &function, const HRESULT hr);
FRAMELESSHELPER_API QString getSystemErrorMessage(const QString &function);
#endif // Q_OS_WINDOWS

#ifdef Q_OS_LINUX
FRAMELESSHELPER_API void sendX11ButtonReleaseEvent(QWindow *w, const QPoint &globalPos);
FRAMELESSHELPER_API void sendX11MoveResizeEvent(QWindow *w, const QPoint &globalPos, int section);
FRAMELESSHELPER_API void startX11Moving(QWindow *w, const QPoint &globalPos);
FRAMELESSHELPER_API void startX11Resizing(QWindow *w, const QPoint &globalPos, Qt::WindowFrameSection frameSection);
FRAMELESSHELPER_API void setX11CursorShape(QWindow *w, int cursorId);
FRAMELESSHELPER_API void resetX1CursorShape(QWindow *w);
FRAMELESSHELPER_API unsigned int getX11CursorForFrameSection(Qt::WindowFrameSection frameSection);
#endif // Q_OS_LINUX

#ifdef Q_OS_MAC
FRAMELESSHELPER_API bool setMacWindowHook(QWindow* w);
FRAMELESSHELPER_API bool unsetMacWindowHook(QWindow* w);
FRAMELESSHELPER_API bool setMacWindowFrameless(QWindow* w);
FRAMELESSHELPER_API bool unsetMacWindowFrameless(QWindow* w);
FRAMELESSHELPER_API bool startMacDrag(QWindow* w, const QPoint& pos);
FRAMELESSHELPER_API Qt::MouseButtons getMacMouseButtons();
FRAMELESSHELPER_API bool setStandardWindowButtonsVisibility(QWindow *w, bool visible);
FRAMELESSHELPER_API bool setStandardWindowButtonsPosition(QWindow *w, const QPoint &pos);
FRAMELESSHELPER_API QSize standardWindowButtonsSize(QWindow *w);
FRAMELESSHELPER_API bool setCloseBtnEnabled(QWindow *w, bool enable = true);
FRAMELESSHELPER_API bool setMinBtnEnabled(QWindow *w, bool enable = true);
FRAMELESSHELPER_API bool setZoomBtnEnabled(QWindow *w, bool enable = true);
#endif // Q_OS_MAC

}

FRAMELESSHELPER_END_NAMESPACE
