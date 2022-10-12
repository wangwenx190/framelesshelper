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

#include "framelesshelperquick_global.h"
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#  include "framelessquickhelper.h"
#  include "framelessquickhelper_p.h"
#  include "framelessquickutils.h"
#  include "quickchromepalette.h"
#  include "quickstandardsystembutton_p.h"
#  include "quickstandardtitlebar_p.h"
#  include "framelessquickwindow_p.h"
#  include "framelessquickwindow_p_p.h"
#  include "framelessquickapplicationwindow_p.h"
#  include "framelessquickapplicationwindow_p_p.h"
#  include "quickmicamaterial.h"
#  include "quickmicamaterial_p.h"
#  include "quickimageitem.h"
#  include "quickimageitem_p.h"
#  include "quickwindowborder.h"
#  include "quickwindowborder_p.h"
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQuickGlobal, "wangwenx190.framelesshelper.quick.global")
#define INFO qCInfo(lcQuickGlobal)
#define DEBUG qCDebug(lcQuickGlobal)
#define WARNING qCWarning(lcQuickGlobal)
#define CRITICAL qCCritical(lcQuickGlobal)

QuickGlobal::QuickGlobal(QObject *parent) : QObject(parent)
{
}

QuickGlobal::~QuickGlobal() = default;

namespace FramelessHelper::Quick
{

void initialize()
{
    static bool inited = false;
    if (inited) {
        return;
    }
    inited = true;

    FramelessHelper::Core::initialize();

    qRegisterMetaType<QuickGlobal::SystemTheme>();
    qRegisterMetaType<QuickGlobal::SystemButtonType>();
#ifdef Q_OS_WINDOWS
    qRegisterMetaType<QuickGlobal::DwmColorizationArea>();
#endif
    qRegisterMetaType<QuickGlobal::ButtonState>();
#ifdef Q_OS_WINDOWS
    qRegisterMetaType<QuickGlobal::WindowsVersion>();
#endif
    qRegisterMetaType<QuickGlobal::ApplicationType>();
    qRegisterMetaType<QuickGlobal::BlurMode>();
    qRegisterMetaType<QuickGlobal::WindowEdge>();
    qRegisterMetaType<QuickGlobal::WindowEdges>();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    qRegisterMetaType<QuickGlobal>();
    qRegisterMetaType<FramelessQuickHelper>();
    qRegisterMetaType<FramelessQuickHelper *>();
    qRegisterMetaType<FramelessQuickHelperPrivate>();
    qRegisterMetaType<FramelessQuickUtils>();
    qRegisterMetaType<FramelessQuickUtils *>();
    qRegisterMetaType<QuickChromePalette>();
    qRegisterMetaType<QuickChromePalette *>();
    qRegisterMetaType<QuickStandardSystemButton>();
    qRegisterMetaType<QuickStandardSystemButton *>();
    qRegisterMetaType<QuickStandardTitleBar>();
    qRegisterMetaType<QuickStandardTitleBar *>();
    qRegisterMetaType<FramelessQuickWindow>();
    qRegisterMetaType<FramelessQuickWindow *>();
    qRegisterMetaType<FramelessQuickWindowPrivate>();
    qRegisterMetaType<FramelessQuickApplicationWindow>();
    qRegisterMetaType<FramelessQuickApplicationWindow *>();
    qRegisterMetaType<FramelessQuickApplicationWindowPrivate>();
    qRegisterMetaType<QuickMicaMaterial>();
    qRegisterMetaType<QuickMicaMaterial *>();
    qRegisterMetaType<QuickMicaMaterialPrivate>();
    qRegisterMetaType<QuickImageItem>();
    qRegisterMetaType<QuickImageItem *>();
    qRegisterMetaType<QuickImageItemPrivate>();
    qRegisterMetaType<QuickWindowBorder>();
    qRegisterMetaType<QuickWindowBorder *>();
    qRegisterMetaType<QuickWindowBorderPrivate>();
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
}

void uninitialize()
{
    static bool uninited = false;
    if (uninited) {
        return;
    }
    uninited = true;

    // ### TODO: The Quick module-specific uninitialization.

    FramelessHelper::Core::uninitialize();
}

}

FRAMELESSHELPER_END_NAMESPACE
