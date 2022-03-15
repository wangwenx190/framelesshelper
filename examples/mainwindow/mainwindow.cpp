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

#include "mainwindow.h"
#include <QtGui/qpainter.h>
#include <QtGui/qevent.h>
#include <framelesswindowsmanager.h>
#include <utilities.h>

FRAMELESSHELPER_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
    setAttribute(Qt::WA_DontCreateNativeAncestors);
    createWinId();
    setupUi();
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
    initFramelessHelperOnce();
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    bool shouldUpdate = false;
    if (event->type() == QEvent::WindowStateChange) {
#ifdef Q_OS_WINDOWS
        if (Utilities::isWindowFrameBorderVisible()) {
            if (isMaximized() || isFullScreen()) {
                setContentsMargins(0, 0, 0, 0);
            } else if (!isMinimized()) {
                resetContentsMargins();
            }
        }
#endif
        shouldUpdate = true;
        Q_EMIT windowStateChanged();
    } else if (event->type() == QEvent::ActivationChange) {
        shouldUpdate = true;
    }
    if (shouldUpdate) {
        update();
    }
}

void MainWindow::initFramelessHelperOnce()
{
    if (m_inited) {
        return;
    }
    m_inited = true;
    FramelessWindowsManager::addWindow(windowHandle());
}

void MainWindow::resetContentsMargins()
{
#ifdef Q_OS_WINDOWS
    if (Utilities::isWindowFrameBorderVisible()) {
        setContentsMargins(0, 1, 0, 0);
    }
#endif
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
#ifdef Q_OS_WINDOWS
    if ((windowState() == Qt::WindowNoState) && Utilities::isWindowFrameBorderVisible() && !Utilities::isWin11OrGreater()) {
        QPainter painter(this);
        painter.save();
        QPen pen = {};
        pen.setColor(Utilities::getFrameBorderColor(isActiveWindow()));
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawLine(0, 0, width(), 0);
        painter.restore();
    }
#endif
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    QMainWindow::mousePressEvent(event);
    const Qt::MouseButton button = event->button();
    if ((button != Qt::LeftButton) && (button != Qt::RightButton)) {
        return;
    }
    if (isInTitleBarDraggableArea(event->pos())) {
        if (button == Qt::LeftButton) {
            Utilities::startSystemMove(windowHandle());
        } else {
#ifdef Q_OS_WINDOWS
#  if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            const QPointF globalPos = event->globalPosition();
#  else
            const QPointF globalPos = event->globalPos();
#  endif
            const QPointF pos = globalPos * devicePixelRatioF();
            Utilities::showSystemMenu(winId(), pos);
#endif
        }
    }
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    QMainWindow::mouseDoubleClickEvent(event);
    if (event->button() != Qt::LeftButton) {
        return;
    }
    if (isInTitleBarDraggableArea(event->pos())) {
        titleBarWidget->maximizeButton->click();
    }
}

void MainWindow::setupUi()
{
    resize(800, 600);

    appMainWindow = new Ui::MainWindow;
    appMainWindow->setupUi(this);

    const auto widget = new QWidget(this);
    titleBarWidget = new Ui::TitleBar;
    titleBarWidget->setupUi(widget);

    QMenuBar *mb = menuBar();
    titleBarWidget->horizontalLayout->insertWidget(1, mb);

    setMenuWidget(widget);

    resetContentsMargins();

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
        const bool check = (isMaximized() || isFullScreen());
        titleBarWidget->maximizeButton->setChecked(check);
        titleBarWidget->maximizeButton->setToolTip(check ? tr("Restore") : tr("Maximize"));
    });

    setWindowTitle(tr("Hello, World!"));
}

bool MainWindow::isInTitleBarDraggableArea(const QPoint &pos) const
{
    QRegion draggableArea = {0, 0, menuWidget()->width(), menuWidget()->height()};
    draggableArea -= titleBarWidget->minimizeButton->geometry();
    draggableArea -= titleBarWidget->maximizeButton->geometry();
    draggableArea -= titleBarWidget->closeButton->geometry();
    return draggableArea.contains(pos);
}
