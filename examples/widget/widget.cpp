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

#include "widget.h"
#include <QtCore/qdatetime.h>
#include <QtCore/qcoreevent.h>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#  include <QtGui/qshortcut.h>
#else
#  include <QtWidgets/qshortcut.h>
#endif
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qfileiconprovider.h>
#include <FramelessHelper/Core/framelessmanager.h>
#include <FramelessHelper/Widgets/framelesswidgetshelper.h>
#include <FramelessHelper/Widgets/standardtitlebar.h>
#include <FramelessHelper/Widgets/standardsystembutton.h>
#include "../shared/settings.h"

extern template void Settings::set<QRect>(const QString &, const QString &, const QRect &);
extern template void Settings::set<qreal>(const QString &, const QString &, const qreal &);

extern template QRect Settings::get<QRect>(const QString &, const QString &);
extern template qreal Settings::get<qreal>(const QString &, const QString &);

FRAMELESSHELPER_USE_NAMESPACE

using namespace Global;

FRAMELESSHELPER_STRING_CONSTANT(Geometry)
FRAMELESSHELPER_STRING_CONSTANT(DevicePixelRatio)

Widget::Widget(QWidget *parent) : FramelessWidget(parent)
{
    initialize();
    m_timerId = startTimer(100);
    connect(FramelessManager::instance(), &FramelessManager::systemThemeChanged, this, &Widget::updateStyleSheet);
}

Widget::~Widget() = default;

void Widget::timerEvent(QTimerEvent *event)
{
    FramelessWidget::timerEvent(event);
    if ((event->timerId() == m_timerId) && m_clockLabel) {
        m_clockLabel->setText(QTime::currentTime().toString(FRAMELESSHELPER_STRING_LITERAL("hh:mm:ss")));
    }
}

void Widget::closeEvent(QCloseEvent *event)
{
    if (!parent()) {
        const QString objName = objectName();
        Settings::set(objName, kGeometry, geometry());
        Settings::set(objName, kDevicePixelRatio, devicePixelRatioF());
    }
    FramelessWidget::closeEvent(event);
}

void Widget::initialize()
{
    setWindowTitle(tr("FramelessHelper demo application - QWidget"));
    setWindowIcon(QFileIconProvider().icon(QFileIconProvider::Computer));
    resize(800, 600);
#if FRAMELESSHELPER_CONFIG(titlebar)
    m_titleBar = new StandardTitleBar(this);
    m_titleBar->setWindowIconVisible(true);
#endif
    m_clockLabel = new QLabel(this);
    m_clockLabel->setFrameShape(QFrame::NoFrame);
    m_clockLabel->setAlignment(Qt::AlignCenter);
    QFont clockFont = font();
    clockFont.setBold(true);
    clockFont.setPointSize(70);
    m_clockLabel->setFont(clockFont);
    m_compilerInfoLabel = new QLabel(this);
    m_compilerInfoLabel->setFrameShape(QFrame::NoFrame);
    m_compilerInfoLabel->setAlignment(Qt::AlignCenter);
    static const VersionInfo versionInfo = FramelessHelperVersion();
    m_compilerInfoLabel->setText(
        FRAMELESSHELPER_STRING_LITERAL("Compiler: %1 %2")
            .arg(QString::fromUtf8(versionInfo.compiler.name),
                 QString::fromUtf8(versionInfo.compiler.version)));
    m_commitInfoLabel = new QLabel(this);
    m_commitInfoLabel->setFrameShape(QFrame::NoFrame);
    m_commitInfoLabel->setAlignment(Qt::AlignCenter);
    m_commitInfoLabel->setText(
        FRAMELESSHELPER_STRING_LITERAL("Commit: %1 (%2)")
            .arg(QString::fromUtf8(versionInfo.commit.hash),
                 QString::fromUtf8(versionInfo.commit.author)));
    const auto clockLabelLayout = new QHBoxLayout;
    clockLabelLayout->setContentsMargins(0, 0, 0, 0);
    clockLabelLayout->setSpacing(0);
    clockLabelLayout->addStretch();
    clockLabelLayout->addWidget(m_clockLabel);
    clockLabelLayout->addStretch();
    const auto compilerInfoLabelLayout = new QHBoxLayout;
    compilerInfoLabelLayout->setContentsMargins(0, 0, 0, 0);
    compilerInfoLabelLayout->setSpacing(0);
    compilerInfoLabelLayout->addStretch();
    compilerInfoLabelLayout->addWidget(m_compilerInfoLabel);
    compilerInfoLabelLayout->addStretch();
    const auto commitInfoLabelLayout = new QHBoxLayout;
    commitInfoLabelLayout->setContentsMargins(0, 0, 0, 0);
    commitInfoLabelLayout->setSpacing(0);
    commitInfoLabelLayout->addStretch();
    commitInfoLabelLayout->addWidget(m_commitInfoLabel);
    commitInfoLabelLayout->addStretch();
    const auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
#if FRAMELESSHELPER_CONFIG(titlebar)
    mainLayout->addWidget(m_titleBar);
#endif
    mainLayout->addStretch();
    mainLayout->addLayout(clockLabelLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(compilerInfoLabelLayout);
    mainLayout->addLayout(commitInfoLabelLayout);
    updateStyleSheet();

    m_cancelShortcut = new QShortcut(this);
    m_cancelShortcut->setKey(FRAMELESSHELPER_STRING_LITERAL("ESC"));
    connect(m_cancelShortcut, &QShortcut::activated, this, [this](){
        if (isFullScreen()) {
            Q_EMIT m_fullScreenShortcut->activated();
        } else {
            close();
        }
    });

    m_fullScreenShortcut = new QShortcut(this);
    m_fullScreenShortcut->setKey(FRAMELESSHELPER_STRING_LITERAL("ALT+RETURN"));
    connect(m_fullScreenShortcut, &QShortcut::activated, this, [this](){
        if (isFullScreen()) {
            setWindowState(windowState() & ~Qt::WindowFullScreen);
        } else {
            showFullScreen();
        }
    });

    connect(this, &Widget::objectNameChanged, this, [this](const QString &name){
        if (name.isEmpty()) {
            return;
        }
        setWindowTitle(windowTitle() + FRAMELESSHELPER_STRING_LITERAL(" [%1]").arg(name));
    });

#if FRAMELESSHELPER_CONFIG(titlebar)
    FramelessWidgetsHelper *helper = FramelessWidgetsHelper::get(this);
    helper->setTitleBarWidget(m_titleBar);
#  if (!defined(Q_OS_MACOS) && FRAMELESSHELPER_CONFIG(system_button))
    helper->setSystemButton(m_titleBar->minimizeButton(), SystemButtonType::Minimize);
    helper->setSystemButton(m_titleBar->maximizeButton(), SystemButtonType::Maximize);
    helper->setSystemButton(m_titleBar->closeButton(), SystemButtonType::Close);
#  endif
#endif
}

void Widget::updateStyleSheet()
{
    const bool dark = (FramelessManager::instance()->systemTheme() == SystemTheme::Dark);
    const QColor labelTextColor = (dark ? kDefaultWhiteColor : kDefaultBlackColor);
    const QString labelStyleSheet = FRAMELESSHELPER_STRING_LITERAL("background-color: transparent; color: %1;").arg(labelTextColor.name());
    m_clockLabel->setStyleSheet(labelStyleSheet);
    m_compilerInfoLabel->setStyleSheet(labelStyleSheet);
    m_commitInfoLabel->setStyleSheet(labelStyleSheet);
    if (FramelessWidgetsHelper::get(this)->isBlurBehindWindowEnabled()) {
        setStyleSheet(FRAMELESSHELPER_STRING_LITERAL("background-color: transparent;"));
    } else {
        const QColor windowBackgroundColor = (dark ? kDefaultSystemDarkColor : kDefaultSystemLightColor);
        setStyleSheet(FRAMELESSHELPER_STRING_LITERAL("background-color: %1;").arg(windowBackgroundColor.name()));
    }
    update();
}

void Widget::waitReady()
{
    FramelessWidgetsHelper *helper = FramelessWidgetsHelper::get(this);
    helper->waitForReady();
    const QString objName = objectName();
    const auto savedGeometry = Settings::get<QRect>(objName, kGeometry);
    if (savedGeometry.isValid() && !parent()) {
        const auto savedDpr = Settings::get<qreal>(objName, kDevicePixelRatio);
        // Qt doesn't support dpr < 1.
        const qreal oldDpr = std::max(savedDpr, qreal(1));
        const qreal scale = (devicePixelRatioF() / oldDpr);
        setGeometry({savedGeometry.topLeft() * scale, savedGeometry.size() * scale});
    } else {
        helper->moveWindowToDesktopCenter();
    }
}
