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
#include <FramelessHelper_Quick>
#include <Utils>

FRAMELESSHELPER_USE_NAMESPACE

int main(int argc, char *argv[])
{
#ifdef Q_OS_WINDOWS
    Utils::tryToEnableHighestDpiAwarenessLevel();
#endif

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);
#endif

    QGuiApplication application(argc, argv);

#ifdef Q_OS_WINDOWS
    if (!qEnvironmentVariableIsSet("QSG_RHI_BACKEND")) {
#  if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
#  elif (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Software);
#  endif
    }
#endif

    QQmlApplicationEngine engine;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QQuickStyle::setStyle(QStringLiteral("Basic"));
#else
    QQuickStyle::setStyle(QStringLiteral("Default"));
#endif

    FramelessHelper::Quick::registerTypes(&engine);

    const QUrl homepageUrl(QStringLiteral("qrc:///qml/MainWindow.qml"));
    const QMetaObject::Connection connection = QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &application,
        [&homepageUrl, &connection](QObject *object, const QUrl &url) {
            if (url != homepageUrl) {
                return;
            }
            if (object) {
                QObject::disconnect(connection);
            } else {
                QCoreApplication::exit(-1);
            }
        }, Qt::QueuedConnection);

    engine.load(homepageUrl);

    return QCoreApplication::exec();
}
