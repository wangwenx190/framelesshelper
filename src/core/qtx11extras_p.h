/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "framelesshelpercore_global.h"
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QtGui/private/qtx11extras_p.h>
#else // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QtCore/private/qglobal_p.h>
#include <QtGui/qtguiglobal.h>

struct xcb_connection_t;
struct xcb_generic_event_t;
struct _XDisplay;

using Display = struct _XDisplay;

QT_BEGIN_NAMESPACE

class FRAMELESSHELPER_CORE_API QX11Info
{
    Q_DISABLE_COPY_MOVE(QX11Info)

public:
    enum class PeekOption
    {
        Default = 0x0000,
        FromCachedIndex = 0x0001
    };
    Q_ENUM(PeekOption)
    Q_DECLARE_FLAGS(PeekOptions, PeekOption)
    Q_FLAG(PeekOptions)

    [[nodiscard]] static bool isPlatformX11();

    [[nodiscard]] static int appDpiX(const int screen = -1);
    [[nodiscard]] static int appDpiY(const int screen = -1);

    [[nodiscard]] static quint32 appRootWindow(const int screen = -1);
    [[nodiscard]] static int appScreen();

    [[nodiscard]] static quint32 appTime();
    [[nodiscard]] static quint32 appUserTime();

    static void setAppTime(const quint32 time);
    static void setAppUserTime(const quint32 time);

    [[nodiscard]] static quint32 getTimestamp();

    [[nodiscard]] static QByteArray nextStartupId();
    static void setNextStartupId(const QByteArray &id);

    [[nodiscard]] static Display *display();
    [[nodiscard]] static xcb_connection_t *connection();

    [[nodiscard]] static bool isCompositingManagerRunning(const int screen = -1);

    [[nodiscard]] static qint32 generatePeekerId();
    [[nodiscard]] static bool removePeekerId(const qint32 peekerId);

    using PeekerCallback = bool(*)(xcb_generic_event_t *, void *);
    [[nodiscard]] static bool peekEventQueue(PeekerCallback peeker, void *peekerData = nullptr,
                               const PeekOptions option = PeekOption::Default, const qint32 peekerId = -1);

private:
    explicit QX11Info();
    ~QX11Info();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QX11Info::PeekOptions)

QT_END_NAMESPACE

#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
