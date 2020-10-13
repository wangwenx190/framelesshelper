/*
 * MIT License
 *
 * Copyright (C) 2020 by wangwenx190 (Yuhang Zhao)
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

#include "../../framelessquickhelper.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[]) {
    // Enable Qt RHI, the default backend on Windows is D3D 11.1
    qputenv("QSG_RHI", "1");
    qputenv("QSG_INFO", "1");
#if (QT_VERSION <= QT_VERSION_CHECK(6, 0, 0))
    QGuiApplication::setAttribute(
        Qt::ApplicationAttribute::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(
        Qt::ApplicationAttribute::AA_UseHighDpiPixmaps);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#endif
    QGuiApplication::setApplicationName(QString::fromUtf8("QQPlayer"));
    QGuiApplication::setApplicationDisplayName(QString::fromUtf8("QQPlayer"));
#ifndef Q_OS_WINDOWS
    QGuiApplication::setApplicationVersion(QString::fromUtf8("1.0.0"));
#endif
    QGuiApplication::setOrganizationName(QString::fromUtf8("wangwenx190"));
    QGuiApplication::setOrganizationDomain(
        QString::fromUtf8("wangwenx190.github.io"));
    QGuiApplication application(argc, argv);
    QQmlApplicationEngine engine;
    qmlRegisterType<FramelessQuickHelper>("wangwenx190.Utils", 1, 0,
                                          "FramelessHelper");
    const QUrl mainQmlUrl(QString::fromUtf8("qrc:/qml/main.qml"));
    const QMetaObject::Connection connection = QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &application,
        [&mainQmlUrl, &connection](QObject *object, const QUrl &url) {
            if (url != mainQmlUrl) {
                return;
            }
            if (!object) {
                QGuiApplication::exit(-1);
            } else {
                QObject::disconnect(connection);
            }
        },
        Qt::QueuedConnection);
    engine.load(mainQmlUrl);
    return QGuiApplication::exec();
}
