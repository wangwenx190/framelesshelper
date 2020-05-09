#include "framelesshelper.h"
#include <QApplication>
#include <QWidget>

int main(int argc, char *argv[]) {
    // High DPI scaling is enabled by default from Qt 6
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#if 0
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::Round);
#else
    // Don't round the scale factor.
    // This will break QWidget applications because they can't render correctly.
    // Qt Quick applications won't have this issue.
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#endif

    QApplication application(argc, argv);

    FramelessHelper helper;

    QWidget widget;
    helper.removeWindowFrame(&widget);
    widget.resize(800, 600);
    FramelessHelper::moveWindowToDesktopCenter(&widget);
    widget.show();

    return QApplication::exec();
}
