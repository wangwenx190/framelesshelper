#include "framelesshelper.h"
#include <QApplication>
#include <QScreen>
#include <QWidget>

static void moveWindowToDesktopCenter(QWidget *const widget) {
    if (widget) {
        const QRect sg = widget->screen()->geometry();
        const int sw = sg.width();
        const int sh = sg.height();
        const int ww = widget->width();
        const int wh = widget->height();
        widget->move(qRound(static_cast<qreal>(sw - ww) / 2.0),
                     qRound(static_cast<qreal>(sh - wh) / 2.0));
    }
}

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
    moveWindowToDesktopCenter(&widget);
    widget.show();

    return QApplication::exec();
}
