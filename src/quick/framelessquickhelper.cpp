﻿/*
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

#include "framelessquickhelper.h"
#include "framelessquickutils.h"
#include "framelessquickwindow.h"
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#  include "quickstandardminimizebutton_p.h"
#  include "quickstandardmaximizebutton_p.h"
#  include "quickstandardclosebutton_p.h"
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

void FramelessHelper::Quick::registerTypes(QQmlEngine *engine)
{
    Q_ASSERT(engine);
    if (!engine) {
        return;
    }
    static bool inited = false;
    if (inited) {
        return;
    }
    inited = true;
    qRegisterMetaType<QuickGlobal::Options>();
    qRegisterMetaType<QuickGlobal::SystemTheme>();
    qRegisterMetaType<QuickGlobal::SystemButtonType>();
    qRegisterMetaType<QuickGlobal::ResourceType>();
    qRegisterMetaType<QuickGlobal::DwmColorizationArea>();
    qRegisterMetaType<QuickGlobal::Anchor>();
    qRegisterMetaType<QuickGlobal::ButtonState>();
#ifdef Q_CC_MSVC
#pragma warning(push)
#pragma warning( disable : 4127 ) // "conditional expression is constant"
#endif
    qmlRegisterUncreatableType<QuickGlobal>(QUICK_URI_FULL, "FramelessHelper",
        FRAMELESSHELPER_STRING_LITERAL("The FramelessHelper namespace is not creatable, you can only use it to access it's enums."));
    qmlRegisterSingletonType<FramelessQuickUtils>(QUICK_URI_EXPAND("FramelessUtils"),
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine);
            Q_UNUSED(scriptEngine);
            return new FramelessQuickUtils;
        });
#ifdef Q_CC_MSVC
#pragma warning(pop)
#endif
    qmlRegisterRevision<QWindow, 254>(QUICK_URI_FULL);
    qmlRegisterRevision<QQuickWindow, 254>(QUICK_URI_FULL);
    qmlRegisterRevision<QQuickItem, 254>(QUICK_URI_FULL);
    qmlRegisterType<FramelessQuickWindow>(QUICK_URI_EXPAND("FramelessWindow"));
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    qmlRegisterType<QuickStandardMinimizeButton>(QUICK_URI_EXPAND("StandardMinimizeButton"));
    qmlRegisterType<QuickStandardMaximizeButton>(QUICK_URI_EXPAND("StandardMaximizeButton"));
    qmlRegisterType<QuickStandardCloseButton>(QUICK_URI_EXPAND("StandardCloseButton"));
    qmlRegisterType<QuickStandardTitleBar>(QUICK_URI_EXPAND("StandardTitleBar"));
#else // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    qmlRegisterTypeNotAvailable(QUICK_URI_EXPAND("StandardMinimizeButton"),
        FRAMELESSHELPER_STRING_LITERAL("StandardMinimizeButton is not available until Qt6."));
    qmlRegisterTypeNotAvailable(QUICK_URI_EXPAND("StandardMaximizeButton"),
        FRAMELESSHELPER_STRING_LITERAL("StandardMaximizeButton is not available until Qt6."));
    qmlRegisterTypeNotAvailable(QUICK_URI_EXPAND("StandardCloseButton"),
        FRAMELESSHELPER_STRING_LITERAL("StandardCloseButton is not available until Qt6."));
    qmlRegisterTypeNotAvailable(QUICK_URI_EXPAND("StandardTitleBar"),
        FRAMELESSHELPER_STRING_LITERAL("StandardTitleBar is not available until Qt6."));
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    qmlRegisterModule(QUICK_URI_FULL);
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
FramelessHelperQuickPlugin::FramelessHelperQuickPlugin(QObject *parent) : QQmlEngineExtensionPlugin(parent)
{
}

FramelessHelperQuickPlugin::~FramelessHelperQuickPlugin() = default;

void FramelessHelperQuickPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_ASSERT(engine);
    Q_ASSERT(uri);
    if (!engine || !uri) {
        return;
    }
    Q_ASSERT(QLatin1String(uri) == QLatin1String(FRAMELESSHELPER_QUICK_URI));
    if (QLatin1String(uri) != QLatin1String(FRAMELESSHELPER_QUICK_URI)) {
        return;
    }
    FramelessHelper::Quick::registerTypes(engine);
}
#endif // (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))

FRAMELESSHELPER_END_NAMESPACE
