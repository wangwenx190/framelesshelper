#include <QtWidgets/qapplication.h>
#include "flwindow.h"

#include <QStyleFactory>

int main(int argc, char *argv[])
{
#if 1
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#endif
#endif
    QApplication application(argc, argv);
    application.setStyle(QStyleFactory::create(QStringLiteral("fusion")));

    FLWindow win;
    win.show();

    return QApplication::exec();
}
