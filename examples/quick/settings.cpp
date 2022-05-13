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

#include "settings.h"
#include <QtCore/qsettings.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <QtCore/qdatastream.h>
#include <QtGui/qwindow.h>

FRAMELESSHELPER_STRING_CONSTANT2(IniKeyPath, "Window/Geometry")

Settings::Settings(QObject *parent) : QObject(parent)
{
    const QFileInfo fileInfo(QCoreApplication::applicationFilePath());
    const QString iniFileName = fileInfo.completeBaseName() + FRAMELESSHELPER_STRING_LITERAL(".ini");
    const QString iniFilePath = fileInfo.canonicalPath() + QDir::separator() + iniFileName;
    m_settings.reset(new QSettings(iniFilePath, QSettings::IniFormat));
}

Settings::~Settings() = default;

void Settings::saveGeometry(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    QByteArray data = {};
    QDataStream stream(&data, QDataStream::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_6);
    stream << window->geometry();
    m_settings->setValue(kIniKeyPath, data);
}

bool Settings::restoreGeometry(QWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
    const QByteArray data = m_settings->value(kIniKeyPath).toByteArray();
    if (data.isEmpty()) {
        return false;
    }
    QRect geometry = {};
    QDataStream stream(data);
    stream.setVersion(QDataStream::Qt_5_6);
    stream >> geometry;
    if (!geometry.isValid()) {
        return false;
    }
    window->setGeometry(geometry);
    return true;
}
