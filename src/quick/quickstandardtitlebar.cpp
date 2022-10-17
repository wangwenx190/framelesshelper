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
#include "quickimageitem.h"
#include "framelessquickhelper.h"
#include "quickstandardsystembutton_p.h"
#include "framelessquickwindow_p.h"
#include <QtCore/qtimer.h>
#include <QtGui/qevent.h>
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickanchors_p.h>
#include <QtQuick/private/qquickanchors_p_p.h>
#include <QtQuick/private/qquickpositioners_p.h>
#include <QtQuickTemplates2/private/qquicktooltip_p.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQuickStandardTitleBar, "wangwenx190.framelesshelper.quick.quickstandardtitlebar")
#define INFO qCInfo(lcQuickStandardTitleBar)
#define DEBUG qCDebug(lcQuickStandardTitleBar)
#define WARNING qCWarning(lcQuickStandardTitleBar)
#define CRITICAL qCCritical(lcQuickStandardTitleBar)

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
        if (m_windowIcon->isVisible()) {
            labelAnchors->setLeft(QQuickItemPrivate::get(m_windowIcon.data())->right());
        } else {
            labelAnchors->setLeft(titleBarPriv->left());
        }
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

QSizeF QuickStandardTitleBar::windowIconSize() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    return m_windowIcon->size();
#else
    return {m_windowIcon->width(), m_windowIcon->height()};
#endif
}

void QuickStandardTitleBar::setWindowIconSize(const QSizeF &value)
{
    Q_ASSERT(!value.isEmpty());
    if (value.isEmpty()) {
        return;
    }
    if (windowIconSize() == value) {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    m_windowIcon->setSize(value);
#else
    m_windowIcon->setWidth(value.width());
    m_windowIcon->setHeight(value.height());
#endif
}

bool QuickStandardTitleBar::windowIconVisible() const
{
    return m_windowIcon->isVisible();
}

void QuickStandardTitleBar::setWindowIconVisible(const bool value)
{
    if (m_windowIcon->isVisible() == value) {
        return;
    }
    m_windowIcon->setVisible(value);
    QQuickAnchors *labelAnchors = QQuickItemPrivate::get(m_windowTitleLabel.data())->anchors();
    if (value) {
        labelAnchors->setLeft(QQuickItemPrivate::get(m_windowIcon.data())->right());
    } else {
        labelAnchors->setLeft(QQuickItemPrivate::get(this)->left());
    }
    FramelessQuickHelper::get(this)->setHitTestVisible(windowIconRect(), windowIconVisible_real());
}

QVariant QuickStandardTitleBar::windowIcon() const
{
    return m_windowIcon->source();
}

void QuickStandardTitleBar::setWindowIcon(const QVariant &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid()) {
        return;
    }
    if (m_windowIcon->source() == value) {
        return;
    }
    m_windowIcon->setSource(value);
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
    const QColor activeForeground = m_chromePalette->titleBarActiveForegroundColor();
    const QColor inactiveForeground = m_chromePalette->titleBarInactiveForegroundColor();
    const QColor normal = m_chromePalette->chromeButtonNormalColor();
    const QColor hover = m_chromePalette->chromeButtonHoverColor();
    const QColor press = m_chromePalette->chromeButtonPressColor();
    m_minimizeButton->setActiveForegroundColor(activeForeground);
    m_minimizeButton->setInactiveForegroundColor(inactiveForeground);
    m_minimizeButton->setNormalColor(normal);
    m_minimizeButton->setHoverColor(hover);
    m_minimizeButton->setPressColor(press);
    m_minimizeButton->updateColor();
    m_maximizeButton->setActiveForegroundColor(activeForeground);
    m_maximizeButton->setInactiveForegroundColor(inactiveForeground);
    m_maximizeButton->setNormalColor(normal);
    m_maximizeButton->setHoverColor(hover);
    m_maximizeButton->setPressColor(press);
    m_maximizeButton->updateColor();
    m_closeButton->setActiveForegroundColor(activeForeground);
    m_closeButton->setInactiveForegroundColor(inactiveForeground);
    m_closeButton->setNormalColor(m_chromePalette->closeButtonNormalColor());
    m_closeButton->setHoverColor(m_chromePalette->closeButtonHoverColor());
    m_closeButton->setPressColor(m_chromePalette->closeButtonPressColor());
    m_closeButton->updateColor();
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

void QuickStandardTitleBar::updateWindowIcon()
{
    // The user has set an icon explicitly, don't override it.
    if (m_windowIcon->source().isValid()) {
        return;
    }
    const QIcon icon = (window() ? window()->icon() : QIcon());
    if (icon.isNull()) {
        return;
    }
    m_windowIcon->setSource(icon);
}

bool QuickStandardTitleBar::mouseEventHandler(QMouseEvent *event)
{
    Q_ASSERT(event);
    if (!event) {
        return false;
    }
    const Qt::MouseButton button = event->button();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QPoint scenePos = event->scenePosition().toPoint();
#else
    const QPoint scenePos = event->windowPos().toPoint();
#endif
    const bool interestArea = isInTitleBarIconArea(scenePos);
    switch (event->type()) {
    case QEvent::MouseButtonRelease:
        if (interestArea) {
            // Sadly the mouse release events are always triggered before the
            // mouse double click events, and if we intercept the mouse release
            // events here, we'll never get the double click events afterwards,
            // so we have to wait for a little bit to make sure the double
            // click events are handled first, before we actually handle the
            // mouse release events here.
            // We need to wait long enough because the time interval between these
            // events is really really short, if the delay time is not long enough,
            // we still can't trigger the double click event due to we have handled
            // the mouse release events here already. But we also can't wait too
            // long, otherwise the system menu will show up too late, which is not
            // a good user experience. In my experiments, I found that 150ms is
            // the minimum value we can use here.
            // We need a copy of the "scenePos" variable here, otherwise it will
            // soon fall out of scope when the lambda function actually runs.
            QTimer::singleShot(150, this, [this, button, scenePos](){
                // The close event is already triggered, don't try to show the
                // system menu anymore, otherwise it will prevent our window
                // from closing.
                if (m_closeTriggered) {
                    return;
                }
                FramelessQuickHelper::get(this)->showSystemMenu([this, button, &scenePos]() -> QPoint {
                    if (button == Qt::LeftButton) {
                        return {0, qRound(height())};
                    }
                    return scenePos;
                }());
            });
            // Don't eat this event, we have not handled it yet.
        }
        break;
    case QEvent::MouseButtonDblClick:
        if (QQuickWindow * const w = window()) {
            if ((button == Qt::LeftButton) && interestArea) {
                m_closeTriggered = true;
                w->close();
                // Eat this event, we have handled it here.
                event->accept();
                return true;
            }
        }
        break;
    default:
        break;
    }
    return false;
}

QRect QuickStandardTitleBar::windowIconRect() const
{
#if 0
    const QSizeF size = windowIconSize();
    const qreal y = ((height() - size.height()) / qreal(2));
    return QRectF(QPointF(kDefaultTitleBarContentsMargin, y), size).toRect();
#else
    return QRectF(QPointF(m_windowIcon->x(), m_windowIcon->y()), windowIconSize()).toRect();
#endif
}

bool QuickStandardTitleBar::windowIconVisible_real() const
{
    if (m_windowIcon.isNull() || !m_windowIcon->isVisible() || !m_windowIcon->source().isValid()) {
        return false;
    }
    return true;
}

bool QuickStandardTitleBar::isInTitleBarIconArea(const QPoint &pos) const
{
    if (!windowIconVisible_real()) {
        return false;
    }
    return windowIconRect().contains(pos);
}

void QuickStandardTitleBar::initialize()
{
    setSmooth(true);
    setClip(true);
    setAntialiasing(true);

    m_chromePalette.reset(new QuickChromePalette(this));
    connect(m_chromePalette.data(), &ChromePalette::titleBarColorChanged,
        this, &QuickStandardTitleBar::updateTitleBarColor);
    connect(m_chromePalette.data(), &ChromePalette::chromeButtonColorChanged,
        this, &QuickStandardTitleBar::updateChromeButtonColor);

    QQuickPen * const b = border();
    b->setWidth(0.0);
    b->setColor(kDefaultTransparentColor);
    setHeight(kDefaultTitleBarHeight);

    const QQuickItemPrivate * const thisPriv = QQuickItemPrivate::get(this);

    m_windowIcon.reset(new QuickImageItem(this));
    QQuickAnchors * const iconAnchors = QQuickItemPrivate::get(m_windowIcon.data())->anchors();
    iconAnchors->setLeft(thisPriv->left());
    iconAnchors->setLeftMargin(kDefaultTitleBarContentsMargin);
    iconAnchors->setVerticalCenter(thisPriv->verticalCenter());
    connect(m_windowIcon.data(), &QuickImageItem::visibleChanged, this, &QuickStandardTitleBar::windowIconVisibleChanged);
    connect(m_windowIcon.data(), &QuickImageItem::sourceChanged, this, &QuickStandardTitleBar::windowIconChanged);
    // ### TODO: QuickImageItem::sizeChanged()
    connect(m_windowIcon.data(), &QuickImageItem::widthChanged, this, &QuickStandardTitleBar::windowIconSizeChanged);
    connect(m_windowIcon.data(), &QuickImageItem::heightChanged, this, &QuickStandardTitleBar::windowIconSizeChanged);

    m_windowTitleLabel.reset(new QQuickLabel(this));
    QFont f = m_windowTitleLabel->font();
    f.setPointSize(kDefaultTitleBarFontPointSize);
    m_windowTitleLabel->setFont(f);

    m_systemButtonsRow.reset(new QQuickRow(this));
    QQuickAnchors * const rowAnchors = QQuickItemPrivate::get(m_systemButtonsRow.data())->anchors();
    rowAnchors->setTop(thisPriv->top());
    rowAnchors->setRight(thisPriv->right());
    m_minimizeButton.reset(new QuickStandardSystemButton(QuickGlobal::SystemButtonType::Minimize, m_systemButtonsRow.data()));
    connect(m_minimizeButton.data(), &QuickStandardSystemButton::clicked, this, &QuickStandardTitleBar::clickMinimizeButton);
    m_maximizeButton.reset(new QuickStandardSystemButton(m_systemButtonsRow.data()));
    connect(m_maximizeButton.data(), &QuickStandardSystemButton::clicked, this, &QuickStandardTitleBar::clickMaximizeButton);
    m_closeButton.reset(new QuickStandardSystemButton(QuickGlobal::SystemButtonType::Close, m_systemButtonsRow.data()));
    connect(m_closeButton.data(), &QuickStandardSystemButton::clicked, this, &QuickStandardTitleBar::clickCloseButton);

    setWindowIconSize(kDefaultWindowIconSize);
    setWindowIconVisible(false);
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
        // The window has changed, we need to re-add or re-remove the window icon rect to
        // the hit test visible whitelist. This is different with Qt Widgets.
        FramelessQuickHelper::get(this)->setHitTestVisible(windowIconRect(), windowIconVisible_real());
    }
}

bool QuickStandardTitleBar::eventFilter(QObject *object, QEvent *event)
{
    Q_ASSERT(object);
    Q_ASSERT(event);
    if (!object || !event) {
        return false;
    }
    if (!object->isWindowType()) {
        return QQuickRectangle::eventFilter(object, event);
    }
    const auto w = qobject_cast<QQuickWindow *>(object);
    if (w != window()) {
        return QQuickRectangle::eventFilter(object, event);
    }
    const QEvent::Type type = event->type();
    if (type == QEvent::LanguageChange) {
        retranslateUi();
        // Don't eat the event here, we need it to keep dispatching to other
        // objects that may be interested in this event.
    } else if ((type >= QEvent::MouseButtonPress) && (type <= QEvent::MouseMove)) {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEventHandler(mouseEvent)) {
            // We have handled the event already, stop dispatching.
            return true;
        }
    }
    return QQuickRectangle::eventFilter(object, event);
}

void QuickStandardTitleBar::updateAll()
{
    updateWindowIcon();
    updateMaximizeButton();
    updateTitleLabelText();
    updateTitleBarColor();
    updateChromeButtonColor();
}

void QuickStandardTitleBar::classBegin()
{
    QQuickRectangle::classBegin();
}

void QuickStandardTitleBar::componentComplete()
{
    QQuickRectangle::componentComplete();
}

FRAMELESSHELPER_END_NAMESPACE
#endif
