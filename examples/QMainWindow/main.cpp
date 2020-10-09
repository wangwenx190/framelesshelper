#include "../../winnativeeventfilter.h"
#include "ui_MainWindow.h"
#include "ui_TitleBar.h"
#include <QApplication>
#include <QWidget>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QApplication application(argc, argv);
    
    QMainWindow* mainWindow = new QMainWindow;
    Ui::MainWindow appMainWindow;
    appMainWindow.setupUi(mainWindow);

    QWidget* widget = new QWidget;
    Ui::TitleBar titleBarWidget;
    titleBarWidget.setupUi(widget);

    titleBarWidget.horizontalLayout->insertWidget(1, mainWindow->menuBar());

    titleBarWidget.iconButton->setIcon(mainWindow->windowIcon());
    titleBarWidget.titleLabel->setText(mainWindow->windowTitle());

    QObject::connect(mainWindow, SIGNAL(windowIconChanged(const QIcon&)), titleBarWidget.iconButton, SLOT(setIcon(const QIcon&)));
    QObject::connect(mainWindow, SIGNAL(windowTitleChanged(const QString&)), titleBarWidget.titleLabel, SLOT(setText(const QString&)));
    QObject::connect(titleBarWidget.closeButton, SIGNAL(clicked()), mainWindow, SLOT(close()));
    QObject::connect(titleBarWidget.minimizeButton, SIGNAL(clicked()), mainWindow, SLOT(showMinimized()));
    QObject::connect(titleBarWidget.maximizeButton, &QPushButton::clicked, [mainWindow, titleBarWidget]() {
        if( mainWindow->isMaximized() )
        {
            mainWindow->showNormal();
            titleBarWidget.maximizeButton->setToolTip(QObject::tr("Maximize"));
        }
        else
        {
            mainWindow->showMaximized();
            titleBarWidget.maximizeButton->setToolTip(QObject::tr("Restore"));
        }
    });

    const auto hWnd_widget = reinterpret_cast<HWND>(mainWindow->winId());
    WinNativeEventFilter::addFramelessWindow(hWnd_widget);
    const auto data_widget = WinNativeEventFilter::windowData(hWnd_widget);
    if( data_widget ) {
        data_widget->ignoreObjects << titleBarWidget.iconButton << titleBarWidget.minimizeButton << titleBarWidget.maximizeButton << titleBarWidget.closeButton;
    }
    mainWindow->setMenuWidget(widget);
    
    mainWindow->show();

    return application.exec();
}

