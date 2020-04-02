#include "winnativeeventfilter.h"

#include <QApplication>
#include <QWidget>

int main(int argc, char *argv[]) {
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication application(argc, argv);

    QWidget widget;
    WinNativeEventFilter::WINDOWDATA data;
    // For testing.
    data.blurEnabled = TRUE;
    WinNativeEventFilter::addFramelessWindow(
        reinterpret_cast<HWND>(widget.winId()), &data);
    widget.show();

    return QApplication::exec();
}
