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

#include "quickstandardtitlebar_p.h"
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include "quickstandardminimizebutton_p.h"
#include "quickstandardmaximizebutton_p.h"
#include "quickstandardclosebutton_p.h"
#include <framelesswindowsmanager.h>
#include <utils.h>
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickanchors_p.h>
#include <QtQuick/private/qquickanchors_p_p.h>
#include <QtQuick/private/qquickpositioners_p.h>
#include <QtQuickTemplates2/private/qquicklabel_p.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

using namespace Global;

QuickStandardTitleBar::QuickStandardTitleBar(QQuickItem *parent) : QQuickRectangle(parent)
{
    initialize();
}

QuickStandardTitleBar::~QuickStandardTitleBar() = default;

bool QuickStandardTitleBar::isActive() const
{
    return m_active;
}

void QuickStandardTitleBar::setActive(const bool value)
{
    if (m_active == value) {
        return;
    }
    m_active = value;
    Q_EMIT activeChanged();
}

bool QuickStandardTitleBar::isMaximized() const
{
    return m_maxBtn->isMaximized();
}

void QuickStandardTitleBar::setMaximized(const bool value)
{
    m_maxBtn->setMaximized(value);
}

Qt::Alignment QuickStandardTitleBar::titleLabelAlignment() const
{
    return m_labelAlignment;
}

void QuickStandardTitleBar::setTitleLabelAlignment(const Qt::Alignment value)
{
    if (m_labelAlignment == value) {
        return;
    }
    m_labelAlignment = value;
    QQuickAnchors * const labelAnchors = QQuickItemPrivate::get(m_label.data())->anchors();
    const QQuickItemPrivate * const titleBarPriv = QQuickItemPrivate::get(this);
    if (m_labelAlignment & Qt::AlignTop) {
        labelAnchors->setTop(titleBarPriv->top());
        labelAnchors->setTopMargin(kDefaultTitleBarTitleLabelMargin);
    }
    if (m_labelAlignment & Qt::AlignBottom) {
        labelAnchors->setBottom(titleBarPriv->bottom());
        labelAnchors->setBottomMargin(kDefaultTitleBarTitleLabelMargin);
    }
    if (m_labelAlignment & Qt::AlignLeft) {
        labelAnchors->setLeft(titleBarPriv->left());
        labelAnchors->setLeftMargin(kDefaultTitleBarTitleLabelMargin);
    }
    if (m_labelAlignment & Qt::AlignRight) {
        labelAnchors->setRight(QQuickItemPrivate::get(m_row.data())->left());
        labelAnchors->setRightMargin(kDefaultTitleBarTitleLabelMargin);
    }
    if (m_labelAlignment & Qt::AlignVCenter) {
        labelAnchors->setTopMargin(0);
        labelAnchors->setBottomMargin(0);
        labelAnchors->setVerticalCenter(titleBarPriv->verticalCenter());
    }
    if (m_labelAlignment & Qt::AlignHCenter) {
        labelAnchors->setLeftMargin(0);
        labelAnchors->setRightMargin(0);
        labelAnchors->setHorizontalCenter(titleBarPriv->horizontalCenter());
    }
    Q_EMIT titleLabelAlignmentChanged();
}

QString QuickStandardTitleBar::title() const
{
    return m_label->text();
}

void QuickStandardTitleBar::setTitle(const QString &value)
{
    m_label->setText(value);
}

QuickStandardMinimizeButton *QuickStandardTitleBar::minimizeButton() const
{
    return m_minBtn.data();
}

QuickStandardMaximizeButton *QuickStandardTitleBar::maximizeButton() const
{
    return m_maxBtn.data();
}

QuickStandardCloseButton *QuickStandardTitleBar::closeButton() const
{
    return m_closeBtn.data();
}

void QuickStandardTitleBar::updateTitleBarColor()
{
    QColor backgroundColor = {};
    QColor foregroundColor = {};
    if (m_active) {
        if (Utils::isTitleBarColorized()) {
#ifdef Q_OS_WINDOWS
            backgroundColor = Utils::getDwmColorizationColor();
#endif
#ifdef Q_OS_LINUX
            backgroundColor = Utils::getWmThemeColor();
#endif
#ifdef Q_OS_MACOS
            backgroundColor = Utils::getControlsAccentColor();
#endif
            foregroundColor = kDefaultWhiteColor;
        } else {
            if (Utils::shouldAppsUseDarkMode()) {
                backgroundColor = kDefaultBlackColor;
                foregroundColor = kDefaultWhiteColor;
            } else {
                backgroundColor = kDefaultWhiteColor;
                foregroundColor = kDefaultBlackColor;
            }
        }
    } else {
        if (Utils::shouldAppsUseDarkMode()) {
            backgroundColor = kDefaultSystemDarkColor;
        } else {
            backgroundColor = kDefaultWhiteColor;
        }
        foregroundColor = kDefaultDarkGrayColor;
    }
    setColor(backgroundColor);
    m_label->setColor(foregroundColor);
}

void QuickStandardTitleBar::initialize()
{
    QQuickPen * const _border = border();
    _border->setWidth(0.0);
    _border->setColor(kDefaultTransparentColor);
    setHeight(kDefaultTitleBarHeight);

    m_label.reset(new QQuickLabel(this));
    QFont f = m_label->font();
    f.setPointSize(kDefaultTitleBarFontPointSize);
    m_label->setFont(f);

    m_row.reset(new QQuickRow(this));
    QQuickAnchors * const rowAnchors = QQuickItemPrivate::get(m_row.data())->anchors();
    const QQuickItemPrivate * const thisPriv = QQuickItemPrivate::get(this);
    rowAnchors->setTop(thisPriv->top());
    rowAnchors->setRight(thisPriv->right());
    m_minBtn.reset(new QuickStandardMinimizeButton(m_row.data()));
    m_maxBtn.reset(new QuickStandardMaximizeButton(m_row.data()));
    m_closeBtn.reset(new QuickStandardCloseButton(m_row.data()));

    connect(FramelessWindowsManager::instance(), &FramelessWindowsManager::systemThemeChanged, this, &QuickStandardTitleBar::updateTitleBarColor);
    connect(this, &QuickStandardTitleBar::activeChanged, this, &QuickStandardTitleBar::updateTitleBarColor);
    connect(m_label.data(), &QQuickLabel::textChanged, this, &QuickStandardTitleBar::titleChanged);
    connect(m_maxBtn.data(), &QuickStandardMaximizeButton::maximizedChanged, this, &QuickStandardTitleBar::maximizedChanged);

    setTitleLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    updateTitleBarColor();
}

FRAMELESSHELPER_END_NAMESPACE
#endif
