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

#include "framelesshelperwidgets_global.h"
#include "standardtitlebar.h"
#include "standardsystembutton.h"
#include "framelesswidgetshelper.h"
#include "framelesswidget.h"
#include "framelessmainwindow.h"
#include "framelessdialog.h"
#include "widgetssharedhelper_p.h"
#include "standardtitlebar_p.h"
#include "standardsystembutton_p.h"
#include "framelesswidgetshelper_p.h"
#include "framelesswidget_p.h"
#include "framelessmainwindow_p.h"
#include "framelessdialog_p.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcWidgetsGlobal, "wangwenx190.framelesshelper.widgets.global")

#ifdef FRAMELESSHELPER_WIDGETS_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcWidgetsGlobal)
#  define DEBUG qCDebug(lcWidgetsGlobal)
#  define WARNING qCWarning(lcWidgetsGlobal)
#  define CRITICAL qCCritical(lcWidgetsGlobal)
#endif

namespace FramelessHelper::Widgets
{

void initialize()
{
    static bool inited = false;
    if (inited) {
        return;
    }
    inited = true;

    FramelessHelper::Core::initialize();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    qRegisterMetaType<StandardTitleBar>();
    qRegisterMetaType<StandardSystemButton>();
    qRegisterMetaType<FramelessWidgetsHelper>();
    qRegisterMetaType<FramelessWidget>();
    qRegisterMetaType<FramelessMainWindow>();
    qRegisterMetaType<FramelessDialog>();
    qRegisterMetaType<WidgetsSharedHelper>();
    qRegisterMetaType<StandardTitleBarPrivate>();
    qRegisterMetaType<StandardSystemButtonPrivate>();
    qRegisterMetaType<FramelessWidgetsHelperPrivate>();
    qRegisterMetaType<FramelessWidgetPrivate>();
    qRegisterMetaType<FramelessMainWindowPrivate>();
    qRegisterMetaType<FramelessDialogPrivate>();
#endif
}

void uninitialize()
{
    static bool uninited = false;
    if (uninited) {
        return;
    }
    uninited = true;

    // ### TODO: The Widgets module-specific uninitialization.

    FramelessHelper::Core::uninitialize();
}

}

FRAMELESSHELPER_END_NAMESPACE
