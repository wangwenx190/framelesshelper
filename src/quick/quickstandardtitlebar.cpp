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
#include "quickstandardsystembutton_p.h"
#include "framelessquickwindow_p.h"
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickanchors_p.h>
#include <QtQuick/private/qquickanchors_p_p.h>
#include <QtQuick/private/qquickpositioners_p.h>
#include <QtQuickTemplates2/private/qquicktooltip_p.h>

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
    QQuickAnchors * const labelAnchors = QQuickItemPrivate::get(m_windowTitleLabel.data())->anchors();
    //labelAnchors->setMargins(0);
    labelAnchors->resetFill();
    labelAnchors->resetCenterIn();
    labelAnchors->resetTop();
    labelAnchors->resetBottom();
    labelAnchors->resetLeft();
    labelAnchors->resetRight();
    const QQuickItemPrivate * const titleBarPriv = QQuickItemPrivate::get(this);
    if (m_labelAlignment & Qt::AlignTop) {
        labelAnchors->setTop(titleBarPriv->top());
        labelAnchors->setTopMargin(kDefaultTitleBarContentsMargin);
    }
    if (m_labelAlignment & Qt::AlignBottom) {
        labelAnchors->setBottom(titleBarPriv->bottom());
        labelAnchors->setBottomMargin(kDefaultTitleBarContentsMargin);
    }
    if (m_labelAlignment & Qt::AlignLeft) {
        labelAnchors->setLeft(titleBarPriv->left());
        labelAnchors->setLeftMargin(kDefaultTitleBarContentsMargin);
    }
    if (m_labelAlignment & Qt::AlignRight) {
        labelAnchors->setRight(QQuickItemPrivate::get(m_systemButtonsRow.data())->left());
        labelAnchors->setRightMargin(kDefaultTitleBarContentsMargin);
    }
    if (m_labelAlignment & Qt::AlignVCenter) {
        //labelAnchors->setTopMargin(0);
        //labelAnchors->setBottomMargin(0);
        labelAnchors->setVerticalCenter(titleBarPriv->verticalCenter());
    }
    if (m_labelAlignment & Qt::AlignHCenter) {
        //labelAnchors->setLeftMargin(0);
        //labelAnchors->setRightMargin(0);
        labelAnchors->setHorizontalCenter(titleBarPriv->horizontalCenter());
    }
    Q_EMIT titleLabelAlignmentChanged();
}

QQuickLabel *QuickStandardTitleBar::titleLabel() const
{
    return m_windowTitleLabel.data();
}

QuickStandardSystemButton *QuickStandardTitleBar::minimizeButton() const
{
    return m_minimizeButton.data();
}

QuickStandardSystemButton *QuickStandardTitleBar::maximizeButton() const
{
    return m_maximizeButton.data();
}

QuickStandardSystemButton *QuickStandardTitleBar::closeButton() const
{
    return m_closeButton.data();
}

bool QuickStandardTitleBar::isExtended() const
{
    return m_extended;
}

void QuickStandardTitleBar::setExtended(const bool value)
{
    if (m_extended == value) {
        return;
    }
    m_extended = value;
    setHeight(m_extended ? kDefaultExtendedTitleBarHeight : kDefaultTitleBarHeight);
    Q_EMIT extendedChanged();
}

bool QuickStandardTitleBar::isHideWhenClose() const
{
    return m_hideWhenClose;
}

void QuickStandardTitleBar::setHideWhenClose(const bool value)
{
    if (m_hideWhenClose == value) {
        return;
    }
    m_hideWhenClose = value;
    Q_EMIT hideWhenCloseChanged();
}

QuickChromePalette *QuickStandardTitleBar::chromePalette() const
{
    return m_chromePalette.data();
}

void QuickStandardTitleBar::updateMaximizeButton()
{
    const QQuickWindow * const w = window();
    if (!w) {
        return;
    }
    const bool max = (w->visibility() == QQuickWindow::Maximized);
    m_maximizeButton->setButtonType(max ? QuickGlobal::SystemButtonType::Restore : QuickGlobal::SystemButtonType::Maximize);
    qobject_cast<QQuickToolTipAttached *>(qmlAttachedPropertiesObject<QQuickToolTip>(m_maximizeButton.data()))->setText(max ? tr("Restore") : tr("Maximize"));
}

void QuickStandardTitleBar::updateTitleLabelText()
{
    const QQuickWindow * const w = window();
    if (!w) {
        return;
    }
    m_windowTitleLabel->setText(w->title());
}

void QuickStandardTitleBar::updateTitleBarColor()
{
    const QQuickWindow * const w = window();
    if (!w) {
        return;
    }
    const bool active = w->isActive();
    const QColor backgroundColor = (active ?
        m_chromePalette->titleBarActiveBackgroundColor() :
        m_chromePalette->titleBarInactiveBackgroundColor());
    const QColor foregroundColor = (active ?
        m_chromePalette->titleBarActiveForegroundColor() :
        m_chromePalette->titleBarInactiveForegroundColor());
    setColor(backgroundColor);
    m_windowTitleLabel->setColor(foregroundColor);
}

void QuickStandardTitleBar::updateChromeButtonColor()
{
    const QQuickWindow * const w = window();
    if (!w) {
        return;
    }
    const bool active = w->isActive();
    const QColor color = (active ?
        m_chromePalette->titleBarActiveForegroundColor() :
        m_chromePalette->titleBarInactiveForegroundColor());
    const QColor normal = m_chromePalette->chromeButtonNormalColor();
    const QColor hover = m_chromePalette->chromeButtonHoverColor();
    const QColor press = m_chromePalette->chromeButtonPressColor();
    m_minimizeButton->setColor(color);
    m_minimizeButton->setNormalColor(normal);
    m_minimizeButton->setHoverColor(hover);
    m_minimizeButton->setPressColor(press);
    m_maximizeButton->setColor(color);
    m_maximizeButton->setNormalColor(normal);
    m_maximizeButton->setHoverColor(hover);
    m_maximizeButton->setPressColor(press);
    m_closeButton->setColor(color);
    // The close button is special.
    m_closeButton->setNormalColor(m_chromePalette->closeButtonNormalColor());
    m_closeButton->setHoverColor(m_chromePalette->closeButtonHoverColor());
    m_closeButton->setPressColor(m_chromePalette->closeButtonPressColor());
}

void QuickStandardTitleBar::clickMinimizeButton()
{
    QQuickWindow * const w = window();
    if (!w) {
        return;
    }
    if (const auto _w = qobject_cast<FramelessQuickWindow *>(w)) {
        _w->showMinimized2();
    } else {
        w->setVisibility(QQuickWindow::Minimized);
    }
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
    if (m_hideWhenClose) {
        w->hide();
    } else {
        w->close();
    }
}

void QuickStandardTitleBar::retranslateUi()
{
    qobject_cast<QQuickToolTipAttached *>(qmlAttachedPropertiesObject<QQuickToolTip>(m_minimizeButton.data()))->setText(tr("Minimize"));
    qobject_cast<QQuickToolTipAttached *>(qmlAttachedPropertiesObject<QQuickToolTip>(m_maximizeButton.data()))->setText([this]() -> QString {
        if (const QQuickWindow * const w = window()) {
            if (w->visibility() == QQuickWindow::Maximized) {
                return tr("Restore");
            }
        }
        return tr("Maximize");
    }());
    qobject_cast<QQuickToolTipAttached *>(qmlAttachedPropertiesObject<QQuickToolTip>(m_closeButton.data()))->setText(tr("Close"));
}

void QuickStandardTitleBar::initialize()
{
    m_chromePalette.reset(new QuickChromePalette(this));
    connect(m_chromePalette.data(), &ChromePalette::titleBarColorChanged,
        this, &QuickStandardTitleBar::updateTitleBarColor);
    connect(m_chromePalette.data(), &ChromePalette::chromeButtonColorChanged,
        this, &QuickStandardTitleBar::updateChromeButtonColor);

    QQuickPen * const b = border();
    b->setWidth(0.0);
    b->setColor(kDefaultTransparentColor);
    setHeight(kDefaultTitleBarHeight);

    m_windowTitleLabel.reset(new QQuickLabel(this));
    QFont f = m_windowTitleLabel->font();
    f.setPointSize(kDefaultTitleBarFontPointSize);
    m_windowTitleLabel->setFont(f);

    m_systemButtonsRow.reset(new QQuickRow(this));
    QQuickAnchors * const rowAnchors = QQuickItemPrivate::get(m_systemButtonsRow.data())->anchors();
    const QQuickItemPrivate * const thisPriv = QQuickItemPrivate::get(this);
    rowAnchors->setTop(thisPriv->top());
    rowAnchors->setRight(thisPriv->right());
    m_minimizeButton.reset(new QuickStandardSystemButton(QuickGlobal::SystemButtonType::Minimize, m_systemButtonsRow.data()));
    connect(m_minimizeButton.data(), &QuickStandardSystemButton::clicked, this, &QuickStandardTitleBar::clickMinimizeButton);
    m_maximizeButton.reset(new QuickStandardSystemButton(m_systemButtonsRow.data()));
    connect(m_maximizeButton.data(), &QuickStandardSystemButton::clicked, this, &QuickStandardTitleBar::clickMaximizeButton);
    m_closeButton.reset(new QuickStandardSystemButton(QuickGlobal::SystemButtonType::Close, m_systemButtonsRow.data()));
    connect(m_closeButton.data(), &QuickStandardSystemButton::clicked, this, &QuickStandardTitleBar::clickCloseButton);

    setTitleLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    retranslateUi();
    updateAll();
}

void QuickStandardTitleBar::itemChange(const ItemChange change, const ItemChangeData &value)
{
    QQuickRectangle::itemChange(change, value);
    if ((change == ItemSceneChange) && value.window) {
        if (m_windowStateChangeConnection) {
            disconnect(m_windowStateChangeConnection);
            m_windowStateChangeConnection = {};
        }
        if (m_windowActiveChangeConnection) {
            disconnect(m_windowActiveChangeConnection);
            m_windowActiveChangeConnection = {};
        }
        if (m_windowTitleChangeConnection) {
            disconnect(m_windowTitleChangeConnection);
            m_windowTitleChangeConnection = {};
        }
        m_windowStateChangeConnection = connect(value.window, &QQuickWindow::visibilityChanged, this, &QuickStandardTitleBar::updateMaximizeButton);
        m_windowActiveChangeConnection = connect(value.window, &QQuickWindow::activeChanged, this, [this](){
            updateTitleBarColor();
            updateChromeButtonColor();
        });
        m_windowTitleChangeConnection = connect(value.window, &QQuickWindow::windowTitleChanged, this, &QuickStandardTitleBar::updateTitleLabelText);
        updateAll();
        value.window->installEventFilter(this);
    }
}

bool QuickStandardTitleBar::eventFilter(QObject *object, QEvent *event)
{
    if (event && (event->type() == QEvent::LanguageChange)) {
        retranslateUi();
    }
    return QQuickRectangle::eventFilter(object, event);
}

void QuickStandardTitleBar::updateAll()
{
    updateMaximizeButton();
    updateTitleLabelText();
    updateTitleBarColor();
    updateChromeButtonColor();
}

FRAMELESSHELPER_END_NAMESPACE
#endif
