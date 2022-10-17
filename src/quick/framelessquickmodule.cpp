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

#include "framelessquickmodule.h"
#include "framelessquickhelper.h"
#include "framelessquickutils.h"
#include "quickchromepalette.h"
#include "quickmicamaterial.h"
#include "quickimageitem.h"
#include "quickwindowborder.h"
#include "framelessquickwindow_p.h"
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#  include "framelessquickapplicationwindow_p.h"
#  include "quickstandardsystembutton_p.h"
#  include "quickstandardtitlebar_p.h"
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

#ifndef QUICK_URI_SHORT
#  define QUICK_URI_SHORT FRAMELESSHELPER_QUICK_URI, 1
#endif

#ifndef QUICK_URI_FULL
#  define QUICK_URI_FULL QUICK_URI_SHORT, 0
#endif

#ifndef QUICK_URI_EXPAND
#  define QUICK_URI_EXPAND(name) QUICK_URI_FULL, name
#endif

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQuickModule, "wangwenx190.framelesshelper.quick.quickmodule")
#define INFO qCInfo(lcQuickModule)
#define DEBUG qCDebug(lcQuickModule)
#define WARNING qCWarning(lcQuickModule)
#define CRITICAL qCCritical(lcQuickModule)

void FramelessHelper::Quick::registerTypes(QQmlEngine *engine)
{
    Q_ASSERT(engine);
    if (!engine) {
        return;
    }

    // In most cases we don't need to register the QML types for multiple times.
    static bool reg = false;
    if (reg) {
        return;
    }
    reg = true;

    // @uri org.wangwenx190.FramelessHelper
    qmlRegisterUncreatableType<QuickGlobal>(QUICK_URI_FULL, "FramelessHelperConstants",
        FRAMELESSHELPER_STRING_LITERAL("The FramelessHelperConstants namespace is not creatable, you can only use it to access it's enums."));
    qmlRegisterSingletonType<FramelessQuickUtils>(QUICK_URI_EXPAND("FramelessUtils"),
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine);
            Q_UNUSED(scriptEngine);
            return new FramelessQuickUtils;
        });
    qmlRegisterAnonymousType<QuickChromePalette>(QUICK_URI_SHORT);

    qmlRegisterType<FramelessQuickHelper>(QUICK_URI_EXPAND("FramelessHelper"));
    qmlRegisterType<FramelessQuickWindow>(QUICK_URI_EXPAND("FramelessWindow"));
    qmlRegisterType<QuickMicaMaterial>(QUICK_URI_EXPAND("MicaMaterial"));
    qmlRegisterType<QuickImageItem>(QUICK_URI_EXPAND("ImageItem"));
    qmlRegisterType<QuickWindowBorder>(QUICK_URI_EXPAND("WindowBorder"));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    qmlRegisterType<FramelessQuickApplicationWindow>(QUICK_URI_EXPAND("FramelessApplicationWindow"));
    qmlRegisterType<QuickStandardSystemButton>(QUICK_URI_EXPAND("StandardSystemButton"));
    qmlRegisterType<QuickStandardTitleBar>(QUICK_URI_EXPAND("StandardTitleBar"));
#else // (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    qmlRegisterTypeNotAvailable(QUICK_URI_EXPAND("FramelessApplicationWindow"),
        FRAMELESSHELPER_STRING_LITERAL("FramelessApplicationWindow is not available until Qt6."));
    qmlRegisterTypeNotAvailable(QUICK_URI_EXPAND("StandardSystemButton"),
        FRAMELESSHELPER_STRING_LITERAL("StandardSystemButton is not available until Qt6."));
    qmlRegisterTypeNotAvailable(QUICK_URI_EXPAND("StandardTitleBar"),
        FRAMELESSHELPER_STRING_LITERAL("StandardTitleBar is not available until Qt6."));
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    qmlRegisterModule(QUICK_URI_FULL);
}

FRAMELESSHELPER_END_NAMESPACE
