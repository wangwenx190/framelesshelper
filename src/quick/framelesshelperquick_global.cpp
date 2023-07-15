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

#include "framelesshelperquick_global.h"
#include <QtCore/qloggingcategory.h>

#define REG_META_TYPE(Type) qRegisterMetaType<Type>(#Type)

FRAMELESSHELPER_BEGIN_NAMESPACE

[[maybe_unused]] static Q_LOGGING_CATEGORY(lcQuickGlobal, "wangwenx190.framelesshelper.quick.global")

#ifdef FRAMELESSHELPER_QUICK_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcQuickGlobal)
#  define DEBUG qCDebug(lcQuickGlobal)
#  define WARNING qCWarning(lcQuickGlobal)
#  define CRITICAL qCCritical(lcQuickGlobal)
#endif

QuickGlobal::QuickGlobal(QObject *parent) : QObject(parent)
{
}

QuickGlobal::~QuickGlobal() = default;

namespace FramelessHelper::Quick
{

void initialize()
{
    static bool inited = false;
    if (inited) {
        return;
    }
    inited = true;

    FramelessHelper::Core::initialize();

    // Registering meta types only causes troubles in Qt6.
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    REG_META_TYPE(QuickGlobal::SystemTheme);
    REG_META_TYPE(QuickGlobal::SystemButtonType);
    REG_META_TYPE(QuickGlobal::ButtonState);
    REG_META_TYPE(QuickGlobal::BlurMode);
    REG_META_TYPE(QuickGlobal::WindowEdge);
#endif
}

void uninitialize()
{
    static bool uninited = false;
    if (uninited) {
        return;
    }
    uninited = true;

    // ### TODO: The Quick module-specific uninitialization.

    FramelessHelper::Core::uninitialize();
}

}

FRAMELESSHELPER_END_NAMESPACE
