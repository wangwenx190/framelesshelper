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
#include "core/utilities.h"

FRAMELESSHELPER_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
    setAttribute(Qt::WA_DontCreateNativeAncestors);
    createWinId();

    resize(800, 600);

    appMainWindow = new Ui::MainWindow;
    appMainWindow->setupUi(this);

    m_titleBar = new QWidget(this);
    titleBarWidget = new Ui::TitleBar;
    titleBarWidget->setupUi(m_titleBar);

    QMenuBar *mb = menuBar();
    titleBarWidget->horizontalLayout->insertWidget(1, mb);
    setMenuWidget(m_titleBar);

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

    connect(appMainWindow->uninstallBtn, &QPushButton::clicked, [this]() {
        this->m_helper->uninstall();
    });
    connect(appMainWindow->installBtn, &QPushButton::clicked, [this]() {
        this->m_helper->install();
    });

    setWindowTitle(tr("Hello, World!"));
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
            m_helper = new FramelessHelper(win);
            m_helper->setHitTestVisible(titleBarWidget->iconButton);
            m_helper->setHitTestVisible(titleBarWidget->minimizeButton);
            m_helper->setHitTestVisible(titleBarWidget->maximizeButton);
            m_helper->setHitTestVisible(titleBarWidget->closeButton);
            m_helper->setHitTestVisible(appMainWindow->menubar);
            m_helper->setTitleBarHeight(m_titleBar->height());
            m_helper->setResizeBorderThickness(4);
            m_helper->install();
#ifndef Q_OS_MAC
            setContentsMargins(1, 1, 1, 1);
#else // Q_OS_MAC
            titleBarWidget->minimizeButton->hide();
            titleBarWidget->maximizeButton->hide();
            titleBarWidget->closeButton->hide();
            Utilities::setStandardWindowButtonsVisibility(windowHandle(), true);
#endif // Q_OS_MAC
            inited = true;
        }
    }
}


#ifdef Q_OS_WIN
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    if (!m_helper)
        return QWidget::nativeEvent(eventType, message, result);

    if (m_helper->handleNativeEvent(this->windowHandle(), eventType, message, result))
        return true;
    else
        return QWidget::nativeEvent(eventType, message, result);
}
#endif // Q_OS_WIN

#ifndef Q_OS_MAC
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
        const ColorizationArea area = Utilities::getColorizationArea();
        const bool colorizedBorder = ((area == ColorizationArea::TitleBar_WindowBorder)
                                      || (area == ColorizationArea::AllArea));
        const QColor borderColor = (isActiveWindow() ? (colorizedBorder ? Utilities::getColorizationColor() : Qt::black) : Qt::darkGray);
        const auto borderThickness = static_cast<qreal>(Utilities::getWindowVisibleFrameBorderThickness(winId()));
        painter.setPen({borderColor, qMax(borderThickness, devicePixelRatioF())});
        painter.drawLines(lines);
        painter.restore();
    }
}
#endif // Q_OS_MAC
