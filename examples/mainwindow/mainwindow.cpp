/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
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

#include "mainwindow.h"
#include <QtGui/qpainter.h>
#include <QSettings>
#include "../../framelesswindowsmanager.h"
#include "../../utilities.h"

FRAMELESSHELPER_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
    setAttribute(Qt::WA_DontCreateNativeAncestors);
    createWinId();

    appMainWindow = new Ui::MainWindow;
    appMainWindow->setupUi(this);

    const auto widget = new QWidget(this);
    titleBarWidget = new Ui::TitleBar;
    titleBarWidget->setupUi(widget);

    QMenuBar *mb = menuBar();
    titleBarWidget->horizontalLayout->insertWidget(1, mb);

    setMenuWidget(widget);

    connect(this, &MainWindow::windowIconChanged, titleBarWidget->iconButton, &QPushButton::setIcon);
    connect(this, &MainWindow::windowTitleChanged, titleBarWidget->titleLabel, &QLabel::setText);
    connect(titleBarWidget->closeButton, &QPushButton::clicked, this, &MainWindow::close);
    connect(titleBarWidget->minimizeButton, &QPushButton::clicked, this, &MainWindow::showMinimized);
    connect(titleBarWidget->maximizeButton, &QPushButton::clicked, this, [this](){
        if (isMaximized() || isFullScreen()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(this, &MainWindow::windowStateChanged, this, [this](){
        titleBarWidget->maximizeButton->setChecked(isMaximized());
        titleBarWidget->maximizeButton->setToolTip(isMaximized() ? tr("Restore") : tr("Maximize"));
    });

    setWindowTitle(tr("Hello, World!"));

    QSettings settings(QLatin1String("FramelessHelper"), QLatin1String("MainWindow"));
    restoreGeometry(settings.value(QLatin1String("geometry")).toByteArray());
    restoreState(settings.value(QLatin1String("windowState")).toByteArray());
}

MainWindow::~MainWindow()
{
    QSettings settings(QLatin1String("FramelessHelper"), QLatin1String("MainWindow"));
    settings.setValue(QLatin1String("geometry"), saveGeometry());
    settings.setValue(QLatin1String("windowState"), saveState());
    if (titleBarWidget) {
        delete titleBarWidget;
        titleBarWidget = nullptr;
    }
    if (appMainWindow) {
        delete appMainWindow;
        appMainWindow = nullptr;
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    static bool inited = false;
    if (!inited) {
        const auto win = windowHandle();
        if (win) {
            FramelessWindowsManager::addWindow(win);
            FramelessWindowsManager::setHitTestVisible(win, titleBarWidget->iconButton, true);
            FramelessWindowsManager::setHitTestVisible(win, titleBarWidget->minimizeButton, true);
            FramelessWindowsManager::setHitTestVisible(win, titleBarWidget->maximizeButton, true);
            FramelessWindowsManager::setHitTestVisible(win, titleBarWidget->closeButton, true);
            FramelessWindowsManager::setHitTestVisible(win, appMainWindow->menubar, true);
            const auto margin = static_cast<int>(qRound(frameBorderThickness()));
            setContentsMargins(margin, margin, margin, margin);
            inited = true;
        }
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    bool shouldUpdate = false;
    if (event->type() == QEvent::WindowStateChange) {
        if (isMaximized() || isFullScreen()) {
            setContentsMargins(0, 0, 0, 0);
        } else if (!isMinimized()) {
            const auto margin = static_cast<int>(qRound(frameBorderThickness()));
            setContentsMargins(margin, margin, margin, margin);
        }
        shouldUpdate = true;
        Q_EMIT windowStateChanged();
    } else if (event->type() == QEvent::ActivationChange) {
        shouldUpdate = true;
    }
    if (shouldUpdate) {
        update();
    }
}

qreal MainWindow::frameBorderThickness() const
{
    return (static_cast<qreal>(Utilities::getWindowVisibleFrameBorderThickness(winId())) / devicePixelRatioF());
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
    if ((windowState() == Qt::WindowNoState)
#ifdef Q_OS_WINDOWS
        && !Utilities::isWin11OrGreater()
#endif
        ) {
        const qreal borderThickness = frameBorderThickness();
        const auto w = static_cast<qreal>(width());
        const auto h = static_cast<qreal>(height());
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        using BorderLines = QList<QLineF>;
#else
        using BorderLines = QVector<QLineF>;
#endif
        const BorderLines lines = {
            {0, 0, w, 0},
            {w - borderThickness, 0, w - borderThickness, h},
            {w, h - borderThickness, 0, h - borderThickness},
            {0, h, 0, 0}
        };
        QPainter painter(this);
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing, false);
        const ColorizationArea area = Utilities::getColorizationArea();
        const bool colorizedBorder = ((area == ColorizationArea::TitleBar_WindowBorder)
                                      || (area == ColorizationArea::All));
        const QColor borderColor = (isActiveWindow() ? (colorizedBorder ? Utilities::getColorizationColor() : Qt::black) : Qt::darkGray);
        painter.setPen({borderColor, borderThickness});
        painter.drawLines(lines);
        painter.restore();
    }
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
#else
bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, long* result)
#endif
{
    if( message ) {
        QPointF pos = {};
        if( Utilities::isSystemMenuRequested(message, &pos) ) {
            if( Utilities::showSystemMenu(winId(), pos) ) {
                return true;
            }
        }
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}