#include "winnativeeventfilter.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMargins>
#include <QPushButton>
#ifdef QT_QUICK_LIB
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#else
#include <QWindow>
#endif
#include <QVBoxLayout>
#include <QWidget>
#include <qpa/qplatformnativeinterface.h>

Q_DECLARE_METATYPE(QMargins)

static const int m_defaultTitleBarHeight = 30;
static const int m_defaultButtonWidth = 45;

static void updateQtFrame(QWindow *const window, const int titleBarHeight) {
    if (window && (titleBarHeight > 0)) {
        // Reduce top frame to zero since we paint it ourselves. Use
        // device pixel to avoid rounding errors.
        const QMargins margins = {0, -titleBarHeight, 0, 0};
        const QVariant marginsVar = QVariant::fromValue(margins);
        // The dynamic property takes effect when creating the platform
        // window.
        window->setProperty("_q_windowsCustomMargins", marginsVar);
        // If a platform window exists, change via native interface.
        QPlatformWindow *platformWindow = window->handle();
        if (platformWindow) {
            QGuiApplication::platformNativeInterface()->setWindowProperty(
                platformWindow, QString::fromUtf8("WindowsCustomMargins"),
                marginsVar);
        }
    }
}

#ifdef QT_QUICK_LIB
class MyQuickView : public QQuickView {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MyQuickView)

public:
    explicit MyQuickView(QWindow *parent = nullptr) : QQuickView(parent) {
        setResizeMode(QQuickView::ResizeMode::SizeRootObjectToView);
    }
    ~MyQuickView() override = default;

protected:
    void resizeEvent(QResizeEvent *event) override {
        QQuickView::resizeEvent(event);
        Q_EMIT windowSizeChanged(event->size());
    }

Q_SIGNALS:
    void windowSizeChanged(const QSize &);
};
#endif

int main(int argc, char *argv[]) {
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
    label->setText(QObject::tr("Hello, World!"));
    QObject::connect(&widget, &QWidget::windowTitleChanged, label,
                     &QLabel::setText);
    QPushButton *minimizeButton = new QPushButton;
    minimizeButton->setText(QObject::tr("Minimize"));
    QObject::connect(minimizeButton, &QPushButton::clicked, &widget,
                     &QWidget::showMinimized);
    QPushButton *maximizeButton = new QPushButton;
    maximizeButton->setText(QObject::tr("Maximize"));
    QObject::connect(maximizeButton, &QPushButton::clicked,
                     [&widget, &maximizeButton]() {
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
    QObject::connect(closeButton, &QPushButton::clicked, &widget,
                     &QWidget::close);
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
    WinNativeEventFilter::WINDOWDATA data_widget;
    data_widget.ignoreObjects << minimizeButton << maximizeButton
                              << closeButton;
    const auto hWnd_widget = reinterpret_cast<HWND>(widget.winId());
    const int tbh_widget = WinNativeEventFilter::getSystemMetric(
        hWnd_widget, WinNativeEventFilter::SystemMetric::TitleBarHeight, false);
    updateQtFrame(widget.windowHandle(),
                  (tbh_widget > 0 ? tbh_widget : m_defaultTitleBarHeight));
    widget.resize(800, 600);
    WinNativeEventFilter::addFramelessWindow(hWnd_widget, &data_widget, true);
    widget.show();

#ifdef QT_QUICK_LIB
    // Qt Quick example:
    MyQuickView view;
    const auto hWnd_qml = reinterpret_cast<HWND>(view.winId());
    const int tbh_qml_sys = WinNativeEventFilter::getSystemMetric(
        hWnd_qml, WinNativeEventFilter::SystemMetric::TitleBarHeight, false);
    const int tbh_qml = tbh_qml_sys > 0 ? tbh_qml_sys : m_defaultTitleBarHeight;
    updateQtFrame(&view, tbh_qml);
    view.rootContext()->setContextProperty(QString::fromUtf8("$TitleBarHeight"),
                                           tbh_qml);
    view.setSource(QUrl(QString::fromUtf8("qrc:///qml/main.qml")));
    QObject::connect(
        &view, &MyQuickView::windowSizeChanged, [hWnd_qml](const QSize &size) {
            const auto data = WinNativeEventFilter::windowData(hWnd_qml);
            if (data) {
                const int tbh_qml = WinNativeEventFilter::getSystemMetric(
                    hWnd_qml,
                    WinNativeEventFilter::SystemMetric::TitleBarHeight, false);
                data->draggableAreas = {
                    {0, 0, (size.width() - (m_defaultButtonWidth * 3)),
                     tbh_qml}};
            }
        });
    const QQuickItem *const rootObject = view.rootObject();
    Q_ASSERT(rootObject);
    // We can't use the Qt5 syntax here because we can't get the function
    // pointers of the signals written in QML.
    QObject::connect(rootObject, SIGNAL(minimizeButtonClicked()), &view,
                     SLOT(showMinimized()));
    QObject::connect(rootObject, SIGNAL(maximizeButtonClicked()), &view,
                     SLOT(showMaximized()));
    QObject::connect(rootObject, SIGNAL(restoreButtonClicked()), &view,
                     SLOT(showNormal()));
    QObject::connect(rootObject, SIGNAL(closeButtonClicked()), &view,
                     SLOT(close()));
    view.resize(800, 600);
    WinNativeEventFilter::addFramelessWindow(hWnd_qml, nullptr, true);
    view.show();
#endif

    return QApplication::exec();
}

#ifdef QT_QUICK_LIB
#include "main_windows.moc"
#endif
