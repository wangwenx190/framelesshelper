/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
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

#include "QtAcrylicMainWindow.h"
#include "utilities.h"
#include "framelesswindowsmanager.h"
#include <QtCore/qdebug.h>
#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>

QtAcrylicMainWindow::QtAcrylicMainWindow(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags) {}

QtAcrylicMainWindow::~QtAcrylicMainWindow() = default;

QColor QtAcrylicMainWindow::tintColor() const
{
    const QColor color = m_acrylicHelper.getTintColor();
    if (color.isValid() && (color != Qt::transparent)) {
        return color;
    } else {
        return palette().color(backgroundRole());
    }
}

void QtAcrylicMainWindow::setTintColor(const QColor &value)
{
    if (!value.isValid()) {
        qWarning() << "Tint color not valid.";
        return;
    }
    if (m_acrylicHelper.getTintColor() != value) {
        m_acrylicHelper.setTintColor(value);
        QPalette pal = palette();
        pal.setColor(backgroundRole(), m_acrylicHelper.getTintColor());
        setPalette(pal);
        //m_acrylicHelper.updateAcrylicBrush(tintColor());
        update();
        Q_EMIT tintColorChanged();
    }
}

qreal QtAcrylicMainWindow::tintOpacity() const
{
    return m_acrylicHelper.getTintOpacity();
}

void QtAcrylicMainWindow::setTintOpacity(const qreal value)
{
    if (m_acrylicHelper.getTintOpacity() != value) {
        m_acrylicHelper.setTintOpacity(value);
        m_acrylicHelper.updateAcrylicBrush(tintColor());
        update();
        Q_EMIT tintOpacityChanged();
    }
}

qreal QtAcrylicMainWindow::noiseOpacity() const
{
    return m_acrylicHelper.getNoiseOpacity();
}

void QtAcrylicMainWindow::setNoiseOpacity(const qreal value)
{
    if (m_acrylicHelper.getNoiseOpacity() != value) {
        m_acrylicHelper.setNoiseOpacity(value);
        m_acrylicHelper.updateAcrylicBrush(tintColor());
        update();
        Q_EMIT noiseOpacityChanged();
    }
}

bool QtAcrylicMainWindow::frameVisible() const
{
    return m_frameVisible;
}

void QtAcrylicMainWindow::setFrameVisible(const bool value)
{
    if (m_frameVisible != value) {
        m_frameVisible = value;
        update();
        Q_EMIT frameVisibleChanged();
    }
}

QColor QtAcrylicMainWindow::frameColor() const
{
    return m_acrylicHelper.getFrameColor();
}

void QtAcrylicMainWindow::setFrameColor(const QColor &value)
{
    if (m_acrylicHelper.getFrameColor() != value) {
        m_acrylicHelper.setFrameColor(value);
        update();
        Q_EMIT frameColorChanged();
    }
}

qreal QtAcrylicMainWindow::frameThickness() const
{
    return m_acrylicHelper.getFrameThickness();
}

void QtAcrylicMainWindow::setFrameThickness(const qreal value)
{
    if (m_acrylicHelper.getFrameThickness() != value) {
        m_acrylicHelper.setFrameThickness(value);
        update();
        Q_EMIT frameThicknessChanged();
    }
}

bool QtAcrylicMainWindow::acrylicEnabled() const
{
    return m_acrylicOn;
}

void QtAcrylicMainWindow::setAcrylicEnabled(const bool value)
{
    m_acrylicOn = value;
    if (m_acrylicOn) {
        setAutoFillBackground(false);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_OpaquePaintEvent);
        setBackgroundRole(QPalette::Base);
    }
}

void QtAcrylicMainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (!acrylicEnabled()) {
        return;
    }

    static bool inited = false;
    if (!inited) {
        const QWindow *win = windowHandle();
        FramelessWindowsManager::addWindow(win);
#if 0
#ifdef Q_OS_WINDOWS
        // TODO: let the user choose what he wants.
        if (Utilities::isWin10OrGreater()) {
            qputenv(_flh_global::_flh_acrylic_forceEnableOfficialMSWin10AcrylicBlur_flag, "True");
        }
#endif
#endif
        Utilities::setBlurEffectEnabled(win, true);
        m_acrylicHelper.install(win);
        m_acrylicHelper.updateAcrylicBrush(tintColor());
        connect(&m_acrylicHelper, &QtAcrylicEffectHelper::needsRepaint, this, qOverload<>(&QtAcrylicMainWindow::update));
        update();
        inited = true;
    }
}

void QtAcrylicMainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if (acrylicEnabled()) {
        m_acrylicHelper.paintWindowBackground(&painter, event->region());
    }
    if (frameVisible()) {
        m_acrylicHelper.paintWindowFrame(&painter);
    }
    QWidget::paintEvent(event);
}

void QtAcrylicMainWindow::changeEvent(QEvent* event)
{
    if( event->type()==QEvent::WindowStateChange )
        Q_EMIT windowStateChanged();
    QMainWindow::changeEvent(event);
}