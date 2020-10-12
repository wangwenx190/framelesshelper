/*
 * MIT License
 *
 * Copyright (C) 2020 by wangwenx190 (Yuhang Zhao)
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

#include "ui_MainWindow.h"
#include "ui_TitleBar.h"
#include <QApplication>
#include <QWidget>
#ifdef Q_OS_WINDOWS
#include "../../winnativeeventfilter.h"
#else
#include "../../framelesshelper.h"
#endif

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#endif

    QApplication application(argc, argv);

#ifndef Q_OS_WINDOWS
    FramelessHelper helper;
#endif

    QMainWindow *mainWindow = new QMainWindow;
    Ui::MainWindow appMainWindow;
    appMainWindow.setupUi(mainWindow);

    QWidget *widget = new QWidget;
    Ui::TitleBar titleBarWidget;
    titleBarWidget.setupUi(widget);

    QMenuBar *menuBar = mainWindow->menuBar();
    titleBarWidget.horizontalLayout->insertWidget(1, menuBar);
    menuBar->setMaximumHeight(20);

    titleBarWidget.iconButton->setIcon(mainWindow->windowIcon());
    titleBarWidget.titleLabel->setText(mainWindow->windowTitle());

    mainWindow->setMenuWidget(widget);

    QObject::connect(mainWindow,
                     &QMainWindow::windowIconChanged,
                     titleBarWidget.iconButton,
                     &QPushButton::setIcon);
    QObject::connect(mainWindow,
                     &QMainWindow::windowTitleChanged,
                     titleBarWidget.titleLabel,
                     &QLabel::setText);
    QObject::connect(titleBarWidget.closeButton,
                     &QPushButton::clicked,
                     mainWindow,
                     &QMainWindow::close);
    QObject::connect(titleBarWidget.minimizeButton,
                     &QPushButton::clicked,
                     mainWindow,
                     &QMainWindow::showMinimized);
    QObject::connect(titleBarWidget.maximizeButton,
                     &QPushButton::clicked,
                     [mainWindow, titleBarWidget]() {
                         if (mainWindow->isMaximized()) {
                             mainWindow->showNormal();
                             titleBarWidget.maximizeButton->setToolTip(QObject::tr("Maximize"));
                         } else {
                             mainWindow->showMaximized();
                             titleBarWidget.maximizeButton->setToolTip(QObject::tr("Restore"));
                         }
                     });

#ifdef Q_OS_WINDOWS
    WinNativeEventFilter::addFramelessWindow(mainWindow);
    const auto data = WinNativeEventFilter::windowData(mainWindow);
    if (data) {
        data->ignoreObjects << titleBarWidget.iconButton << titleBarWidget.minimizeButton
                            << titleBarWidget.maximizeButton << titleBarWidget.closeButton
                            << appMainWindow.menubar;
    }
#else
    helper.setIgnoreObjects(mainWindow,
                            {titleBarWidget.iconButton,
                             titleBarWidget.minimizeButton,
                             titleBarWidget.maximizeButton,
                             titleBarWidget.closeButton,
                             appMainWindow.menubar});
    helper.removeWindowFrame(mainWindow);
#endif

    mainWindow->resize(800, 600);
#ifdef Q_OS_WINDOWS
    WinNativeEventFilter::moveWindowToDesktopCenter(mainWindow);
#else
    FramelessHelper::moveWindowToDesktopCenter(mainWindow);
#endif

    mainWindow->show();

    return QApplication::exec();
}
