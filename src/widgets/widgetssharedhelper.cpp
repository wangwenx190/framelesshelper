/*
 * MIT License
 *
 * Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)
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

#include "widgetssharedhelper_p.h"
#include <QtCore/qcoreevent.h>
#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include <QtWidgets/qwidget.h>
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

WidgetsSharedHelper::WidgetsSharedHelper(QObject *parent) : QObject(parent)
{
}

WidgetsSharedHelper::~WidgetsSharedHelper() = default;

void WidgetsSharedHelper::setup(QWidget *widget)
{
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    if (m_targetWidget == widget) {
        return;
    }
    m_targetWidget = widget;
    m_targetWidget->installEventFilter(this);
    updateContentsMargins();
    m_targetWidget->update();
}

bool WidgetsSharedHelper::eventFilter(QObject *object, QEvent *event)
{
    Q_ASSERT(object);
    Q_ASSERT(event);
    if (!object || !event) {
        return false;
    }
    if (!object->isWidgetType()) {
        return QObject::eventFilter(object, event);
    }
    const auto widget = qobject_cast<QWidget *>(object);
    if (widget != m_targetWidget) {
        return QObject::eventFilter(object, event);
    }
    switch (event->type()) {
    case QEvent::Paint: {
        const auto paintEvent = static_cast<QPaintEvent *>(event);
        paintEventHandler(paintEvent);
    } break;
    case QEvent::WindowStateChange: {
        changeEventHandler(event);
    } break;
    default:
        break;
    }
    return QObject::eventFilter(object, event);
}

void WidgetsSharedHelper::changeEventHandler(QEvent *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    if (event->type() != QEvent::WindowStateChange) {
        return;
    }
    updateContentsMargins();
    QMetaObject::invokeMethod(m_targetWidget, "hiddenChanged");
    QMetaObject::invokeMethod(m_targetWidget, "normalChanged");
    QMetaObject::invokeMethod(m_targetWidget, "zoomedChanged");
}

void WidgetsSharedHelper::paintEventHandler(QPaintEvent *event)
{
#ifdef Q_OS_WINDOWS
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    if (!shouldDrawFrameBorder()) {
        return;
    }
    QPainter painter(m_targetWidget);
    painter.save();
    QPen pen = {};
    pen.setColor(Utils::getFrameBorderColor(m_targetWidget->isActiveWindow()));
    pen.setWidth(kDefaultWindowFrameBorderThickness);
    painter.setPen(pen);
    // In fact, we should use "m_targetWidget->width() - 1" here but we can't
    // because Qt's drawing system has some rounding errors internally and if
    // we minus one here we'll get a one pixel gap, so sad. But drawing a line
    // with a little extra pixels won't hurt anyway.
    painter.drawLine(0, 0, m_targetWidget->width(), 0);
    painter.restore();
#else
    Q_UNUSED(event);
#endif
}

bool WidgetsSharedHelper::shouldDrawFrameBorder() const
{
#ifdef Q_OS_WINDOWS
    static const bool isWin11OrGreater = Utils::isWindowsVersionOrGreater(WindowsVersion::_11_21H2);
    return (Utils::isWindowFrameBorderVisible() && !isWin11OrGreater
            && (Utils::windowStatesToWindowState(m_targetWidget->windowState()) == Qt::WindowNoState));
#else
    return false;
#endif
}

void WidgetsSharedHelper::updateContentsMargins()
{
#ifdef Q_OS_WINDOWS
    m_targetWidget->setContentsMargins(0, (shouldDrawFrameBorder() ? kDefaultWindowFrameBorderThickness : 0), 0, 0);
#endif
}

FRAMELESSHELPER_END_NAMESPACE
