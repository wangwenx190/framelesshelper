/*
 * MIT License
 *
 * Copyright (C) 2021-2023 by wangwenx190 (Yuhang Zhao)
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

#include <QtWidgets/qapplication.h>
#include <FramelessHelper/Core/private/framelessconfig_p.h>
#include "mainwindow.h"
#include "../shared/log.h"

FRAMELESSHELPER_USE_NAMESPACE

#define CREATE_WINDOW(Name) \
    const auto Name = std::make_unique<MainWindow>(); \
    Name->setObjectName(FRAMELESSHELPER_STRING_LITERAL(#Name)); \
    Name->waitReady(); \
    Name->show();

int main(int argc, char *argv[])
{
    Log::setup(FRAMELESSHELPER_STRING_LITERAL("mainwindow"));

    // Not necessary, but better call this function, before the construction
    // of any Q(Core|Gui)Application instances.
    FramelessHelperWidgetsInitialize();

#if 0
    if (!qEnvironmentVariableIsSet("QT_WIDGETS_RHI")) {
        qputenv("QT_WIDGETS_RHI", FRAMELESSHELPER_BYTEARRAY_LITERAL("1"));
    }
#endif

    const auto application = std::make_unique<QApplication>(argc, argv);

    // Must be called after QGuiApplication has been constructed, we are using
    // some private functions from QPA which won't be available until there's
    // a QGuiApplication instance.
    FramelessHelperEnableThemeAware();

    FramelessConfig::instance()->set(Global::Option::EnableBlurBehindWindow);
    //FramelessConfig::instance()->set(Global::Option::DisableLazyInitializationForMicaMaterial);

    CREATE_WINDOW(mainWindow1)
    CREATE_WINDOW(mainWindow2)
    CREATE_WINDOW(mainWindow3)

    return QCoreApplication::exec();
}
