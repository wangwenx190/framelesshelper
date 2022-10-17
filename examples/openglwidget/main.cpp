/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QDebug>
#include <QApplication>
#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <framelessconfig_p.h>
#include <clocale>
#include "mainwindow.h"
#include "../shared/log.h"

// This example demonstrates easy, cross-platform usage of OpenGL ES 3.0 functions via
// QOpenGLExtraFunctions in an application that works identically on desktop platforms
// with OpenGL 3.3 and mobile/embedded devices with OpenGL ES 3.0.

// The code is always the same, with the exception of two places: (1) the OpenGL context
// creation has to have a sufficiently high version number for the features that are in
// use, and (2) the shader code's version directive is different.

FRAMELESSHELPER_USE_NAMESPACE

int main(int argc, char *argv[])
{
    std::setlocale(LC_ALL, "en_US.UTF-8");

    Log::setup(FRAMELESSHELPER_STRING_LITERAL("openglwidget"));

    // Not necessary, but better call this function, before the construction
    // of any Q(Core|Gui)Application instances.
    FramelessHelper::Widgets::initialize();

    const QScopedPointer<QApplication> application(new QApplication(argc, argv));

    // Must be called after QGuiApplication has been constructed, we are using
    // some private functions from QPA which won't be available until there's
    // a QGuiApplication instance.
    FramelessHelper::Core::setApplicationOSThemeAware();

    FramelessConfig::instance()->set(Global::Option::WindowUseRoundCorners);
    FramelessConfig::instance()->set(Global::Option::EnableBlurBehindWindow);
    FramelessConfig::instance()->set(Global::Option::DisableLazyInitializationForMicaMaterial);

    QSurfaceFormat fmt = {};
    fmt.setDepthBufferSize(24);

    // Request OpenGL 3.3 core or OpenGL ES 3.0.
    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) {
        qDebug() << "Requesting OpenGL 3.3 core context ...";
        fmt.setVersion(3, 3);
        fmt.setProfile(QSurfaceFormat::CoreProfile);
    } else {
        qDebug() << "Requesting OpenGL ES 3.0 context ...";
        fmt.setVersion(3, 0);
    }

    QSurfaceFormat::setDefaultFormat(fmt);

    const QScopedPointer<MainWindow> mainWindow(new MainWindow);
    mainWindow->show();

    const int exec = QCoreApplication::exec();

    // Not necessary, but if you don't call it, there will be some small memory leaks.
    FramelessHelper::Widgets::uninitialize();

    return exec;
}
