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

#include "framelesshelper_quick.h"
#include <QtQml/qqmlengine.h>
#include "framelesshelperimageprovider.h"
#include "framelessquickutils.h"
#include "framelessquickwindow.h"

#ifndef QML_URL_EXPAND
#  define QML_URL_EXPAND(fileName) \
     QUrl(FRAMELESSHELPER_STRING_LITERAL("qrc:///org.wangwenx190.FramelessHelper/qml/%1.qml") \
        .arg(FRAMELESSHELPER_STRING_LITERAL(fileName)))
#endif

#ifndef QUICK_URI_SHORT
#  define QUICK_URI_SHORT FRAMELESSHELPER_QUICK_URI, 1
#endif

#ifndef QUICK_URI_EXPAND
#  define QUICK_URI_EXPAND(name) QUICK_URI_SHORT, 0, name
#endif

// The "Q_INIT_RESOURCE()" macro can't be used inside a namespace,
// the official workaround is to wrap it into a global function
// and call the wrapper function inside the namespace.
static inline void initResource()
{
    Q_INIT_RESOURCE(framelesshelperquick);
}

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
    engine->addImageProvider(FRAMELESSHELPER_STRING_LITERAL("framelesshelper"), new FramelessHelperImageProvider);
    qmlRegisterUncreatableMetaObject(Global::staticMetaObject, QUICK_URI_EXPAND("FramelessHelper"),
        FRAMELESSHELPER_STRING_LITERAL("The FramelessHelper namespace is not creatable, you can only use it to access its enums."));
    qmlRegisterSingletonType<FramelessQuickUtils>(QUICK_URI_EXPAND("FramelessUtils"),
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine);
            Q_UNUSED(scriptEngine);
            return new FramelessQuickUtils;
        });
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    qmlRegisterAnonymousType<QWindow, 254>(QUICK_URI_SHORT);
#else
    qmlRegisterAnonymousType<QWindow>(QUICK_URI_SHORT);
#endif
    qmlRegisterType<FramelessQuickWindow>(QUICK_URI_EXPAND("FramelessWindow"));
    initResource();
    qmlRegisterType(QML_URL_EXPAND("MinimizeButton"), QUICK_URI_EXPAND("MinimizeButton"));
    qmlRegisterType(QML_URL_EXPAND("MaximizeButton"), QUICK_URI_EXPAND("MaximizeButton"));
    qmlRegisterType(QML_URL_EXPAND("CloseButton"), QUICK_URI_EXPAND("CloseButton"));
    qmlRegisterType(QML_URL_EXPAND("StandardTitleBar"), QUICK_URI_EXPAND("StandardTitleBar"));
}

FRAMELESSHELPER_END_NAMESPACE
