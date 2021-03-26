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

#include "../../framelesswindowsmanager.h"
#include "../../qtacrylicmainwindow.h"
#include "ui_MainWindow.h"
#include "ui_TitleBar.h"
#include <QApplication>
#include <QStyleOption>
#include <QWidget>
#include <QWindow>
#ifdef WIN32
#include <Windows.h>
#endif

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    // Don't round the scale factor.
    // This will break QWidget applications because they can't render correctly.
    // Qt Quick applications won't have this issue.
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#endif

    QApplication application(argc, argv);

    QtAcrylicMainWindow *mainWindow = new QtAcrylicMainWindow(NULL, Qt::WindowFlags());
    mainWindow->setAcrylicEnabled(true);

    Ui::MainWindow appMainWindow;
    appMainWindow.setupUi(mainWindow);

    QWidget *widget = new QWidget;
    Ui::TitleBar titleBarWidget;
    titleBarWidget.setupUi(widget);

    QMenuBar *menuBar = mainWindow->menuBar();
    titleBarWidget.horizontalLayout->insertWidget(1, menuBar);

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
                         } else {
                             mainWindow->showMaximized();
                         }
                     });

    QStyleOption option;
    option.initFrom(mainWindow);
    const QIcon icon = mainWindow->style()->standardIcon(QStyle::SP_ComputerIcon, &option);
    mainWindow->setWindowIcon(icon);
    mainWindow->setWindowTitle(QObject::tr("Hello, World!"));

    mainWindow->createWinId(); // Qt's internal function, make sure it's a top level window.
    const QWindow *win = mainWindow->windowHandle();
    
    QObject::connect(mainWindow,
                     &QtAcrylicMainWindow::windowStateChanged,
                     [mainWindow, titleBarWidget]() {
                        titleBarWidget.maximizeButton->setChecked(mainWindow->isMaximized());
                        titleBarWidget.maximizeButton->setToolTip(mainWindow->isMaximized() ? QObject::tr("Restore") : QObject::tr("Maximize"));
                     });

    QObject::connect(titleBarWidget.iconButton,
                     &QPushButton::clicked,
                     mainWindow,
                     &QtAcrylicMainWindow::displaySystemMenu);

    FramelessWindowsManager::addIgnoreObject(win, titleBarWidget.iconButton);
    FramelessWindowsManager::addIgnoreObject(win, titleBarWidget.minimizeButton);
    FramelessWindowsManager::addIgnoreObject(win, titleBarWidget.maximizeButton);
    FramelessWindowsManager::addIgnoreObject(win, titleBarWidget.closeButton);
    FramelessWindowsManager::addIgnoreObject(win, appMainWindow.menubar);

    mainWindow->resize(800, 600);
    Q_EMIT mainWindow->windowStateChanged();

    mainWindow->show();

    return QApplication::exec();
}
