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
#include "ui_MainWindow.h"
#include "ui_TitleBar.h"

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags) : FramelessMainWindow(parent, flags)
{
    setupUi();
}

MainWindow::~MainWindow()
{
    if (titleBar) {
        delete titleBar;
        titleBar = nullptr;
    }
    if (mainWindow) {
        delete mainWindow;
        mainWindow = nullptr;
    }
}

void MainWindow::setupUi()
{
    mainWindow = new Ui::MainWindow;
    mainWindow->setupUi(this);

    const auto titleBarWidget = new QWidget(this);
    titleBar = new Ui::TitleBar;
    titleBar->setupUi(titleBarWidget);

    QMenuBar *mb = menuBar();
    titleBar->horizontalLayout->insertWidget(1, mb);

    setMenuWidget(titleBarWidget);

    setTitleBarWidget(titleBarWidget);

    setHitTestVisible(titleBar->iconButton, true);
    setHitTestVisible(titleBar->minimizeButton, true);
    setHitTestVisible(titleBar->maximizeButton, true);
    setHitTestVisible(titleBar->closeButton, true);

    connect(titleBar->minimizeButton, &QPushButton::clicked, this, &MainWindow::showMinimized);
    connect(titleBar->maximizeButton, &QPushButton::clicked, this, [this](){
        if (isZoomed()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(titleBar->closeButton, &QPushButton::clicked, this, &MainWindow::close);
    connect(this, &MainWindow::windowIconChanged, titleBar->iconButton, &QPushButton::setIcon);
    connect(this, &MainWindow::windowTitleChanged, titleBar->titleLabel, &QLabel::setText);
    connect(this, &MainWindow::windowStateChanged, this, [this](){
        const bool zoomed = isZoomed();
        titleBar->maximizeButton->setChecked(zoomed);
        titleBar->maximizeButton->setToolTip(zoomed ? tr("Restore") : tr("Maximize"));
    });
}
