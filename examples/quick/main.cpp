/*
 * MIT License
 *
 * Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "../../utilities.h"
#include "../../framelessquickhelper.h"
#include <QtGui/qguiapplication.h>
#include <QtQml/qqmlapplicationengine.h>
#include <QtQuickControls2/qquickstyle.h>

FRAMELESSHELPER_USE_NAMESPACE

static constexpr const char qtquicknamespace[] = "wangwenx190.Utils";

class UtilFunctions : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(UtilFunctions)
    Q_PROPERTY(bool isWindowsHost READ isWindowsHost CONSTANT)
    Q_PROPERTY(bool isWindows10OrGreater READ isWindows10OrGreater CONSTANT)
    Q_PROPERTY(bool isWindows11OrGreater READ isWindows11OrGreater CONSTANT)
    Q_PROPERTY(QColor activeFrameBorderColor READ activeFrameBorderColor CONSTANT)
    Q_PROPERTY(QColor inactiveFrameBorderColor READ inactiveFrameBorderColor CONSTANT)
    Q_PROPERTY(qreal frameBorderThickness READ frameBorderThickness CONSTANT)

public:
    explicit UtilFunctions(QObject *parent = nullptr) : QObject(parent) {}
    ~UtilFunctions() override = default;

    inline bool isWindowsHost() const {
#ifdef Q_OS_WINDOWS
        return true;
#else
        return false;
#endif
    }

    inline bool isWindows10OrGreater() const {
#ifdef Q_OS_WINDOWS
        return Utilities::isWin10OrGreater();
#else
        return false;
#endif
    }

    inline bool isWindows11OrGreater() const {
#ifdef Q_OS_WINDOWS
        return Utilities::isWin11OrGreater();
#else
        return false;
#endif
    }

    inline QColor activeFrameBorderColor() const {
        const ColorizationArea area = Utilities::getColorizationArea();
        const bool colorizedBorder = ((area == ColorizationArea::TitleBar_WindowBorder)
                                      || (area == ColorizationArea::All));
        return (colorizedBorder ? Utilities::getColorizationColor() : Qt::black);
    }

    inline QColor inactiveFrameBorderColor() const {
        return Qt::darkGray;
    }

    inline qreal frameBorderThickness() const {
        return 1.0;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);
#endif

    QGuiApplication application(argc, argv);

    QQmlApplicationEngine engine;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QQuickStyle::setStyle(QStringLiteral("Basic"));
#else
    QQuickStyle::setStyle(QStringLiteral("Default"));
#endif

    qmlRegisterSingletonType<UtilFunctions>(qtquicknamespace, 1, 0, "Utils", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine);
        Q_UNUSED(scriptEngine);
        return new UtilFunctions();
    });
    qmlRegisterType<FramelessQuickHelper>(qtquicknamespace, 1, 0, "FramelessHelper");

    const QUrl mainQmlUrl(QStringLiteral("qrc:///qml/main.qml"));
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

    return QGuiApplication::exec();
}

#include "main.moc"
