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
#include "framelessquickhelper.h"
#include "framelessquickutils.h"

// The "Q_INIT_RESOURCE()" macro can't be used inside a namespace,
// the official workaround is to wrap it into a global function
// and call the wrapper function inside the namespace.
static inline void initResource()
{
    Q_INIT_RESOURCE(framelesshelperquick);
}

FRAMELESSHELPER_BEGIN_NAMESPACE

[[nodiscard]] static inline QUrl getQmlFileUrl(const QString &qml)
{
    Q_ASSERT(!qml.isEmpty());
    if (qml.isEmpty()) {
        return {};
    }
    return QUrl(QStringLiteral("qrc:///org.wangwenx190.FramelessHelper/qml/%1.qml").arg(qml));
}

void FramelessHelper::Quick::registerTypes(QQmlEngine *engine)
{
    Q_ASSERT(engine);
    if (!engine) {
        return;
    }
    engine->addImageProvider(QStringLiteral("framelesshelper"), new FramelessHelperImageProvider);
    qmlRegisterSingletonType<FramelessQuickHelper>(FRAMELESSHELPER_QUICK_URI, 1, 0, "FramelessHelper", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine);
        Q_UNUSED(scriptEngine);
        return new FramelessQuickHelper;
    });
    qmlRegisterSingletonType<FramelessQuickUtils>(FRAMELESSHELPER_QUICK_URI, 1, 0, "FramelessUtils", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine);
        Q_UNUSED(scriptEngine);
        return new FramelessQuickUtils;
    });
    initResource();
    qmlRegisterType(getQmlFileUrl(QStringLiteral("MinimizeButton")), FRAMELESSHELPER_QUICK_URI, 1, 0, "MinimizeButton");
    qmlRegisterType(getQmlFileUrl(QStringLiteral("MaximizeButton")), FRAMELESSHELPER_QUICK_URI, 1, 0, "MaximizeButton");
    qmlRegisterType(getQmlFileUrl(QStringLiteral("CloseButton")), FRAMELESSHELPER_QUICK_URI, 1, 0, "CloseButton");
    qmlRegisterType(getQmlFileUrl(QStringLiteral("StandardTitleBar")), FRAMELESSHELPER_QUICK_URI, 1, 0, "StandardTitleBar");
}

FRAMELESSHELPER_END_NAMESPACE
