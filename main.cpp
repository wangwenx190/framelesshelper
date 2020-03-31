#include "winnativeeventfilter.h"

#include <QApplication>
#include <QWidget>

int main(int argc, char *argv[]) {
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication application(argc, argv);

    WinNativeEventFilter::install();

    QWidget widget;
    widget.show();

    return QApplication::exec();
}
