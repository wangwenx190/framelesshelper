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

void QuickStandardTitleBar::updateMaximizeButton()
{
    const QQuickWindow * const w = window();
    if (!w) {
        return;
    }
    m_maxBtn->setMaximized(w->visibility() == QQuickWindow::Maximized);
}

void QuickStandardTitleBar::updateTitleLabelText()
{
    const QQuickWindow * const w = window();
    if (!w) {
        return;
    }
    m_label->setText(w->title());
}

void QuickStandardTitleBar::updateTitleBarColor()
{
    const QQuickWindow * const w = window();
    if (!w) {
        return;
    }
    QColor backgroundColor = {};
    QColor foregroundColor = {};
    if (w->isActive()) {
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

void QuickStandardTitleBar::clickMinimizeButton()
{
    QQuickWindow * const w = window();
    if (!w) {
        return;
    }
    w->setVisibility(QQuickWindow::Minimized);
}

void QuickStandardTitleBar::clickMaximizeButton()
{
    QQuickWindow * const w = window();
    if (!w) {
        return;
    }
    if (w->visibility() == QQuickWindow::Maximized) {
        w->setVisibility(QQuickWindow::Windowed);
    } else {
        w->setVisibility(QQuickWindow::Maximized);
    }
}

void QuickStandardTitleBar::clickCloseButton()
{
    QQuickWindow * const w = window();
    if (!w) {
        return;
    }
    w->close();
}

void QuickStandardTitleBar::initialize()
{
    QQuickPen * const b = border();
    b->setWidth(0.0);
    b->setColor(kDefaultTransparentColor);
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
    connect(m_minBtn.data(), &QuickStandardMinimizeButton::clicked, this, &QuickStandardTitleBar::clickMinimizeButton);
    m_maxBtn.reset(new QuickStandardMaximizeButton(m_row.data()));
    connect(m_maxBtn.data(), &QuickStandardMaximizeButton::clicked, this, &QuickStandardTitleBar::clickMaximizeButton);
    m_closeBtn.reset(new QuickStandardCloseButton(m_row.data()));
    connect(m_closeBtn.data(), &QuickStandardCloseButton::clicked, this, &QuickStandardTitleBar::clickCloseButton);

    connect(FramelessWindowsManager::instance(), &FramelessWindowsManager::systemThemeChanged, this, &QuickStandardTitleBar::updateTitleBarColor);

    setTitleLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    updateAll();
}

void QuickStandardTitleBar::itemChange(const ItemChange change, const ItemChangeData &value)
{
    QQuickRectangle::itemChange(change, value);
    if ((change == ItemSceneChange) && value.window) {
        if (m_windowStateChangeConnection) {
            disconnect(m_windowStateChangeConnection);
        }
        if (m_windowActiveChangeConnection) {
            disconnect(m_windowActiveChangeConnection);
        }
        if (m_windowTitleChangeConnection) {
            disconnect(m_windowTitleChangeConnection);
        }
        m_windowStateChangeConnection = connect(value.window, &QQuickWindow::visibilityChanged, this, &QuickStandardTitleBar::updateMaximizeButton);
        m_windowActiveChangeConnection = connect(value.window, &QQuickWindow::activeChanged, this, &QuickStandardTitleBar::updateTitleBarColor);
        m_windowTitleChangeConnection = connect(value.window, &QQuickWindow::windowTitleChanged, this, &QuickStandardTitleBar::updateTitleLabelText);
        updateAll();
    }
}

void QuickStandardTitleBar::updateAll()
{
    updateMaximizeButton();
    updateTitleLabelText();
    updateTitleBarColor();
}

FRAMELESSHELPER_END_NAMESPACE
#endif
