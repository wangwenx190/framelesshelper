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
#include "../../framelesswindowsmanager.h"

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
    connect(titleBarWidget->maximizeButton, &QPushButton::clicked, [this](){
        if (isMaximized() || isFullScreen()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(this, &MainWindow::windowStateChanged, [this](){
        titleBarWidget->maximizeButton->setChecked(isMaximized());
        titleBarWidget->maximizeButton->setToolTip(isMaximized() ? tr("Restore") : tr("Maximize"));
    });
}

MainWindow::~MainWindow()
{
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
            FramelessWindowsManager::setHitTestVisibleInChrome(win, titleBarWidget->iconButton, true);
            FramelessWindowsManager::setHitTestVisibleInChrome(win, titleBarWidget->minimizeButton, true);
            FramelessWindowsManager::setHitTestVisibleInChrome(win, titleBarWidget->maximizeButton, true);
            FramelessWindowsManager::setHitTestVisibleInChrome(win, titleBarWidget->closeButton, true);
            FramelessWindowsManager::setHitTestVisibleInChrome(win, appMainWindow->menubar, true);
            setContentsMargins(1, 1, 1, 1);
            inited = true;
        }
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    bool shouldUpdate = false;
    if (event->type() == QEvent::WindowStateChange) {
        if (isMaximized() || isFullScreen()) {
            setContentsMargins(0, 0, 0, 0);
        } else if (!isMinimized()) {
            setContentsMargins(1, 1, 1, 1);
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

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
    if (windowState() == Qt::WindowNoState) {
        QPainter painter(this);
        const int w = width();
        const int h = height();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        using BorderLines = QList<QLine>;
#else
        using BorderLines = QVector<QLine>;
#endif
        const BorderLines lines = {
            {0, 0, w, 0},
            {w - 1, 0, w - 1, h},
            {w, h - 1, 0, h - 1},
            {0, h, 0, 0}
        };
        painter.save();
        painter.setPen({isActiveWindow() ? Qt::black : Qt::darkGray, 1});
        painter.drawLines(lines);
        painter.restore();
    }
}
