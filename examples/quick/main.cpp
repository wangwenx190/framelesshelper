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

#ifndef QMLTC_ENABLED
#  define QMLTC_ENABLED 0
#endif

#include <QtGui/qguiapplication.h>
#include <QtQml/qqmlapplicationengine.h>
#include <QtQml/qqmlcontext.h>
#include <QtQuick/qquickwindow.h>
#include <FramelessHelper/Quick/framelessquickmodule.h>
#include <FramelessHelper/Core/private/framelessconfig_p.h>
#include "quicksettings.h"
#if QMLTC_ENABLED
#  include <homepage.h>
#endif
#include "../shared/log.h"

FRAMELESSHELPER_REQUIRE_CONFIG(window)

FRAMELESSHELPER_USE_NAMESPACE

static constexpr const bool IS_MACOS_HOST =
#ifdef Q_OS_MACOS
        true
#else // !Q_OS_MACOS
        false
#endif // Q_OS_MACOS
        ;

int main(int argc, char *argv[])
{
    Log::setup(FRAMELESSHELPER_STRING_LITERAL("quick"));

    // Not necessary, but better call this function, before the construction
    // of any Q(Core|Gui)Application instances.
    FramelessHelperQuickInitialize();

    const auto application = std::make_unique<QGuiApplication>(argc, argv);

    // Must be called after QGuiApplication has been constructed, we are using
    // some private functions from QPA which won't be available until there's
    // a QGuiApplication instance.
    FramelessHelperEnableThemeAware();

    FramelessConfig::instance()->set(Global::Option::EnableBlurBehindWindow);
    //FramelessConfig::instance()->set(Global::Option::DisableLazyInitializationForMicaMaterial);

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

    if (!qEnvironmentVariableIsSet("QT_QUICK_CONTROLS_STYLE")) {
        // This line is not relevant to FramelessHelper, we change the default
        // Qt Quick Controls theme to "Basic" (Qt6) or "Default" (Qt5) just
        // because other themes will make our homemade system buttons look
        // not good. This line has nothing to do with FramelessHelper itself.
        qputenv("QT_QUICK_CONTROLS_STYLE", []() -> QByteArray {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            return FRAMELESSHELPER_BYTEARRAY_LITERAL("Basic");
#else
            return FRAMELESSHELPER_BYTEARRAY_LITERAL("Default");
#endif
        }());
    }

#if 0
    // Enable some helpful debugging messages.
    if (!qEnvironmentVariableIsSet("QML_IMPORT_TRACE")) {
        qputenv("QML_IMPORT_TRACE", FRAMELESSHELPER_BYTEARRAY_LITERAL("1"));
    }
#endif

    const auto engine = std::make_unique<QQmlApplicationEngine>();

    engine->rootContext()->setContextProperty(
        FRAMELESSHELPER_STRING_LITERAL("$isMacOSHost"), QVariant(IS_MACOS_HOST));

#if (((QT_VERSION < QT_VERSION_CHECK(6, 2, 0)) || defined(QUICK_USE_QMAKE)) && !QMLTC_ENABLED)
    // Don't forget to register our own custom QML types!
    FramelessHelperQuickRegisterTypes(engine.get());

    qmlRegisterSingletonType<QuickSettings>("Demo", 1, 0, "Settings",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine);
            Q_UNUSED(scriptEngine);
            return new QuickSettings;
        });
#endif

#if !QMLTC_ENABLED
    const QUrl mainUrl(FRAMELESSHELPER_STRING_LITERAL("qrc:///qml/HomePage.qml"));
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
    QObject::connect(engine.get(), &QQmlApplicationEngine::objectCreationFailed, application.get(),
        [](const QUrl &url){
            qCritical() << "The QML engine failed to create component:" << url;
            QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
#elif !QMLTC_ENABLED
    const QMetaObject::Connection connection = QObject::connect(
        engine.get(), &QQmlApplicationEngine::objectCreated, application.get(),
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
#endif

#if !QMLTC_ENABLED
    engine->load(mainUrl);
#endif

#if QMLTC_ENABLED
    const auto homePage = std::make_unique<HomePage>(engine.get());
    homePage->show();
#endif

    return QCoreApplication::exec();
}
