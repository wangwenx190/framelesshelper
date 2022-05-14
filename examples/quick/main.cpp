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

#include <QtGui/qguiapplication.h>
#include <QtQml/qqmlapplicationengine.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuickControls2/qquickstyle.h>
#include <framelessquickmodule.h>
#include "settings.h"

FRAMELESSHELPER_USE_NAMESPACE

int main(int argc, char *argv[])
{
    // Not necessary, but better call this function, before the construction
    // of any Q(Core|Gui)Application instances.
    FramelessHelper::Core::initialize();

    QGuiApplication application(argc, argv);

    // Enable QtRHI debug output if not explicitly requested by the user.
    if (!qEnvironmentVariableIsSet("QSG_INFO")) {
        qputenv("QSG_INFO", FRAMELESSHELPER_BYTEARRAY_LITERAL("1"));
    }

    // Allow testing other RHI backends through environment variable.
    if (!qEnvironmentVariableIsSet("QSG_RHI_BACKEND")) {
        // This line is not relevant to FramelessHelper, we change
        // the default RHI backend to "software" just because it's
        // stable enough and have exactly the same behavior on all
        // supported platforms. Other backends may behave differently
        // on different platforms and graphics cards, so I think they
        // are not suitable for our demonstration.
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Software);
#endif
    }

    QQmlApplicationEngine engine;

    // Don't forget to register our own custom QML types!
    FramelessHelper::Quick::registerTypes(&engine);

    qmlRegisterSingletonType<Settings>("Demo", 1, 0, "Settings",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine);
            Q_UNUSED(scriptEngine);
            return new Settings;
        });

    // This line is not relevant to FramelessHelper, we change the default
    // Qt Quick Controls theme to "Basic" (Qt6) or "Default" (Qt5) just
    // because other themes will make our homemade system buttons look
    // not good. This line has nothing to do with FramelessHelper itself.
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QQuickStyle::setStyle(FRAMELESSHELPER_STRING_LITERAL("Basic"));
#else
    QQuickStyle::setStyle(FRAMELESSHELPER_STRING_LITERAL("Default"));
#endif

    const QUrl mainUrl(FRAMELESSHELPER_STRING_LITERAL("qrc:///Demo/qml/MainWindow.qml"));
    const QMetaObject::Connection connection = QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &application,
        [&mainUrl, &connection](QObject *object, const QUrl &url) {
            if (url != mainUrl) {
                return;
            }
            if (object) {
                QObject::disconnect(connection);
            } else {
                QCoreApplication::exit(-1);
            }
        }, Qt::QueuedConnection);

    engine.load(mainUrl);

    return QCoreApplication::exec();
}
