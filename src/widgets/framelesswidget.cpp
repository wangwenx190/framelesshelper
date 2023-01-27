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

#include "framelesswidget.h"
#include "framelesswidget_p.h"
#include "framelesswidgetshelper.h"
#include "widgetssharedhelper_p.h"
#include <FramelessHelper/Core/utils.h>
#include <QtCore/qloggingcategory.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

[[maybe_unused]] static Q_LOGGING_CATEGORY(lcFramelessWidget, "wangwenx190.framelesshelper.widgets.framelesswidget")

#ifdef FRAMELESSHELPER_WIDGETS_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcFramelessWidget)
#  define DEBUG qCDebug(lcFramelessWidget)
#  define WARNING qCWarning(lcFramelessWidget)
#  define CRITICAL qCCritical(lcFramelessWidget)
#endif

using namespace Global;

FramelessWidgetPrivate::FramelessWidgetPrivate(FramelessWidget *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
    initialize();
}

FramelessWidgetPrivate::~FramelessWidgetPrivate() = default;

FramelessWidgetPrivate *FramelessWidgetPrivate::get(FramelessWidget *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

const FramelessWidgetPrivate *FramelessWidgetPrivate::get(const FramelessWidget *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

void FramelessWidgetPrivate::initialize()
{
    Q_Q(FramelessWidget);
    FramelessWidgetsHelper::get(q)->extendsContentIntoTitleBar();
    m_sharedHelper = new WidgetsSharedHelper(this);
    m_sharedHelper->setup(q);
}

bool FramelessWidgetPrivate::isNormal() const
{
    Q_Q(const FramelessWidget);
    return (Utils::windowStatesToWindowState(q->windowState()) == Qt::WindowNoState);
}

bool FramelessWidgetPrivate::isZoomed() const
{
    Q_Q(const FramelessWidget);
    return (q->isMaximized() || q->isFullScreen());
}

void FramelessWidgetPrivate::toggleMaximized()
{
    Q_Q(FramelessWidget);
    if (q->isMaximized()) {
        q->showNormal();
    } else {
        q->showMaximized();
    }
}

void FramelessWidgetPrivate::toggleFullScreen()
{
    Q_Q(FramelessWidget);
    if (q->isFullScreen()) {
        q->setWindowState(m_savedWindowState);
    } else {
        m_savedWindowState = Utils::windowStatesToWindowState(q->windowState());
        q->showFullScreen();
    }
}

WidgetsSharedHelper *FramelessWidgetPrivate::widgetsSharedHelper() const
{
    return m_sharedHelper;
}

FramelessWidget::FramelessWidget(QWidget *parent)
    : QWidget(parent, Qt::Window),  d_ptr(new FramelessWidgetPrivate(this))
{
}

FramelessWidget::~FramelessWidget() = default;

bool FramelessWidget::isNormal() const
{
    Q_D(const FramelessWidget);
    return d->isNormal();
}

bool FramelessWidget::isZoomed() const
{
    Q_D(const FramelessWidget);
    return d->isZoomed();
}

void FramelessWidget::toggleMaximized()
{
    Q_D(FramelessWidget);
    d->toggleMaximized();
}

void FramelessWidget::toggleFullScreen()
{
    Q_D(FramelessWidget);
    d->toggleFullScreen();
}

FRAMELESSHELPER_END_NAMESPACE
