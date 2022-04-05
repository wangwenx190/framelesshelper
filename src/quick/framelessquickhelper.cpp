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

#include "framelessquickhelper.h"
#include <QtQml/qqmlextensionplugin.h>
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

#ifndef QUICK_URI_FULL
#  define QUICK_URI_FULL QUICK_URI_SHORT, 0
#endif

#ifndef QUICK_URI_EXPAND
#  define QUICK_URI_EXPAND(name) QUICK_URI_FULL, name
#endif

#ifndef qmlRegisterAnonymousType2
#  if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#    define qmlRegisterAnonymousType2(Class, ...) qmlRegisterAnonymousType<Class, 254>(__VA_ARGS__)
#  else
#    define qmlRegisterAnonymousType2(Class, ...) qmlRegisterAnonymousType<Class>(__VA_ARGS__)
#  endif
#endif

#ifndef qmlRegisgerFile
#  define qmlRegisterFile(Name) qmlRegisterType(QML_URL_EXPAND(Name), QUICK_URI_EXPAND(Name))
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
    qmlRegisterModule(QUICK_URI_FULL);
    qmlRegisterUncreatableMetaObject(Global::staticMetaObject, QUICK_URI_EXPAND("FramelessHelper"),
        FRAMELESSHELPER_STRING_LITERAL("The FramelessHelper namespace is not creatable, you can only use it to access its enums."));
    qmlRegisterSingletonType<FramelessQuickUtils>(QUICK_URI_EXPAND("FramelessUtils"),
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine);
            Q_UNUSED(scriptEngine);
            return new FramelessQuickUtils;
        });
    qmlRegisterAnonymousType2(QWindow, QUICK_URI_SHORT);
    qmlRegisterAnonymousType2(QQuickWindow, QUICK_URI_SHORT);
    qmlRegisterType<FramelessQuickWindow>(QUICK_URI_EXPAND("FramelessWindow"));
    initResource();
    qmlRegisterFile("StandardMinimizeButton");
    qmlRegisterFile("StandardMaximizeButton");
    qmlRegisterFile("StandardCloseButton");
    qmlRegisterFile("StandardTitleBar");
}

class FRAMELESSHELPER_QUICK_API FramelessHelperExtensionPlugin : public QQmlEngineExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlEngineExtensionInterface_iid)

public:
    void initializeEngine(QQmlEngine *engine, const char *uri) override
    {
        Q_ASSERT(engine);
        Q_ASSERT(uri);
        if (!engine || !uri) {
            return;
        }
        if (qstrcmp(uri, FRAMELESSHELPER_QUICK_URI) != 0) {
            return;
        }
        FramelessHelper::Quick::registerTypes(engine);
    }
};

FRAMELESSHELPER_END_NAMESPACE

#include "framelessquickhelper.moc"
