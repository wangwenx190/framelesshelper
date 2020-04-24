#include <QApplication>
#include <QWidget>
#include <QWindow>

#ifdef Q_OS_WINDOWS
#include "winnativeeventfilter.h"
#else
#include "framelesshelper.h"
#endif

int main(int argc, char *argv[]) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#if 0
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::Round);
#else
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#endif

    QApplication application(argc, argv);

#ifdef Q_OS_WINDOWS
    // Do this AFTER the Q(Gui)Application is constructed!
    WinNativeEventFilter::install();
#else
    FramelessHelper helper;
#endif

    QWindow window;
    QWidget widget;
#ifndef Q_OS_WINDOWS
    helper.setFramelessWindows({&widget});
#endif
    window.show();
    widget.show();

    return QApplication::exec();
}
