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
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
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
