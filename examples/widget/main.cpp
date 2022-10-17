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

#include <QtWidgets/qapplication.h>
#include <framelessconfig_p.h>
#include <clocale>
#include "widget.h"
#include "../shared/log.h"

FRAMELESSHELPER_USE_NAMESPACE

int main(int argc, char *argv[])
{
    std::setlocale(LC_ALL, "en_US.UTF-8");

    Log::setup(FRAMELESSHELPER_STRING_LITERAL("widget"));

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

    const QScopedPointer<Widget> window1(new Widget);
    window1->setObjectName(FRAMELESSHELPER_STRING_LITERAL("window1"));
    window1->show();

    const QScopedPointer<Widget> window2(new Widget);
    window2->setObjectName(FRAMELESSHELPER_STRING_LITERAL("window2"));
    window2->show();

    const int exec = QCoreApplication::exec();

    // Not necessary, but if you don't call it, there will be some small memory leaks.
    FramelessHelper::Widgets::uninitialize();

    return exec;
}
