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

#include "../../framelesswindowsmanager.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

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
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#endif

    QApplication application(argc, argv);

    QWidget widget;
    widget.setContentsMargins(2, 2, 2, 2);
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

    FramelessWindowsManager::addWindow(&widget);

    FramelessWindowsManager::addIgnoreObject(&widget, minimizeButton);
    FramelessWindowsManager::addIgnoreObject(&widget, maximizeButton);
    FramelessWindowsManager::addIgnoreObject(&widget, closeButton);

    widget.resize(800, 600);

    FramelessWindowsManager::moveWindowToDesktopCenter(&widget);

    widget.show();

    return QApplication::exec();
}
