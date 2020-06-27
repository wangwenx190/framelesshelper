#include "winnativeeventfilter.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#ifdef QT_QUICK_LIB
#include "framelessquickhelper.h"
#include <QQmlApplicationEngine>
#endif
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[])
{
    // High DPI scaling is enabled by default from Qt 6
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    // Windows: we are using the manifest file to get maximum compatibility
    // because some APIs are not supprted on old systems such as Windows 7
    // and Windows 8. And once we have set the DPI awareness level in the
    // manifest file, any attemptation to try to change it through API will
    // fail. In other words, Qt won't be able to enable or disable high DPI
    // scaling or change the DPI awareness level once we have set it in the
    // manifest file. So the following two lines are uesless actually (However,
    // they are still useful on other platforms).
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

    // Qt Widgets example:
    QWidget widget;
    widget.setContentsMargins(0, 0, 0, 0);
    QLabel *label = new QLabel;
    QObject::connect(&widget, &QWidget::windowTitleChanged, label, &QLabel::setText);
    QPushButton *minimizeButton = new QPushButton;
    minimizeButton->setText(QObject::tr("Minimize"));
    QObject::connect(minimizeButton, &QPushButton::clicked, &widget, &QWidget::showMinimized);
    QPushButton *maximizeButton = new QPushButton;
    maximizeButton->setText(QObject::tr("Maximize"));
    QObject::connect(maximizeButton, &QPushButton::clicked, [&widget, &maximizeButton]() {
        if (widget.isMaximized()) {
            widget.showNormal();
            maximizeButton->setText(QObject::tr("Maximize"));
        } else {
            widget.showMaximized();
            maximizeButton->setText(QObject::tr("Restore"));
        }
    });
    QPushButton *closeButton = new QPushButton;
    closeButton->setText(QObject::tr("Close"));
    QObject::connect(closeButton, &QPushButton::clicked, &widget, &QWidget::close);
    QHBoxLayout *tbLayout = new QHBoxLayout;
    tbLayout->setContentsMargins(0, 0, 0, 0);
    tbLayout->setSpacing(0);
    tbLayout->addSpacing(15);
    tbLayout->addWidget(label);
    tbLayout->addStretch();
    tbLayout->addWidget(minimizeButton);
    tbLayout->addWidget(maximizeButton);
    tbLayout->addWidget(closeButton);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addLayout(tbLayout);
    mainLayout->addStretch();
    widget.setLayout(mainLayout);
    widget.setWindowTitle(QObject::tr("Hello, World!"));
    const auto hWnd_widget = reinterpret_cast<HWND>(widget.winId());
    WinNativeEventFilter::addFramelessWindow(hWnd_widget);
    const auto data_widget = WinNativeEventFilter::windowData(hWnd_widget);
    if (data_widget) {
        data_widget->ignoreObjects << minimizeButton << maximizeButton << closeButton;
    }
    widget.resize(800, 600);
    WinNativeEventFilter::moveWindowToDesktopCenter(hWnd_widget);
    widget.show();

#ifdef QT_QUICK_LIB
    // Qt Quick example:
    QQmlApplicationEngine engine;
    qmlRegisterType<FramelessQuickHelper>("wangwenx190.Utils", 1, 0, "FramelessHelper");
    const QUrl mainQmlUrl(QString::fromUtf8("qrc:///qml/main.qml"));
    const QMetaObject::Connection connection = QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &application,
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
#endif

    return QApplication::exec();
}
