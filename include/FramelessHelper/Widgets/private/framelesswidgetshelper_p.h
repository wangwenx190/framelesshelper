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

#pragma once

#include <FramelessHelper/Widgets/framelesshelperwidgets_global.h>
#include <QtCore/qvariant.h>
#include <QtWidgets/qsizepolicy.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

class FramelessWidgetsHelper;
struct FramelessWidgetsHelperData;
class WidgetsSharedHelper;
class MicaMaterial;
class WindowBorderPainter;

class FRAMELESSHELPER_WIDGETS_API FramelessWidgetsHelperPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(FramelessWidgetsHelper)
    Q_DISABLE_COPY_MOVE(FramelessWidgetsHelperPrivate)

public:
    explicit FramelessWidgetsHelperPrivate(FramelessWidgetsHelper *q);
    ~FramelessWidgetsHelperPrivate() override;

    Q_NODISCARD static FramelessWidgetsHelperPrivate *get(FramelessWidgetsHelper *pub);
    Q_NODISCARD static const FramelessWidgetsHelperPrivate *get(const FramelessWidgetsHelper *pub);

    Q_NODISCARD bool isContentExtendedIntoTitleBar() const;
    void extendsContentIntoTitleBar(const bool value);

    Q_NODISCARD QWidget *getTitleBarWidget() const;
    void setTitleBarWidget(QWidget *widget);

    void attach();
    void detach();
    void setSystemButton(QWidget *widget, const Global::SystemButtonType buttonType);
    void setHitTestVisible(QWidget *widget, const bool visible = true);
    void setHitTestVisible(const QRect &rect, const bool visible = true);
    void setHitTestVisible(QObject *object, const bool visible = true);
    void showSystemMenu(const QPoint &pos);
    void windowStartSystemMove2(const QPoint &pos);
    void windowStartSystemResize2(const Qt::Edges edges, const QPoint &pos);

    void moveWindowToDesktopCenter();
    void bringWindowToFront();

    Q_NODISCARD bool isWindowFixedSize() const;
    void setWindowFixedSize(const bool value);

    void emitSignalForAllInstances(const char *signal);

    Q_NODISCARD bool isBlurBehindWindowEnabled() const;
    void setBlurBehindWindowEnabled(const bool enable, const QColor &color);

    void setProperty(const char *name, const QVariant &value);
    Q_NODISCARD QVariant getProperty(const char *name, const QVariant &defaultValue = {});

    Q_NODISCARD QWidget *window() const;

    Q_NODISCARD MicaMaterial *getMicaMaterialIfAny() const;
    Q_NODISCARD WindowBorderPainter *getWindowBorderIfAny() const;

    Q_NODISCARD static WidgetsSharedHelper *findOrCreateSharedHelper(QWidget *window);
    Q_NODISCARD static FramelessWidgetsHelper *findOrCreateFramelessHelper(QObject *object);

    Q_NODISCARD bool isReady() const;
    void waitForReady();

    void repaintAllChildren(const quint32 delay = 0) const;

    Q_NODISCARD quint32 readyWaitTime() const;
    void setReadyWaitTime(const quint32 time);

private:
    Q_NODISCARD QRect mapWidgetGeometryToScene(const QWidget * const widget) const;
    Q_NODISCARD bool isInSystemButtons(const QPoint &pos, Global::SystemButtonType *button) const;
    Q_NODISCARD bool isInTitleBarDraggableArea(const QPoint &pos) const;
    Q_NODISCARD bool shouldIgnoreMouseEvents(const QPoint &pos) const;
    void setSystemButtonState(const Global::SystemButtonType button, const Global::ButtonState state);
    Q_NODISCARD QWidget *findTopLevelWindow() const;
    Q_NODISCARD const FramelessWidgetsHelperData *getWindowData() const;
    Q_NODISCARD FramelessWidgetsHelperData *getWindowDataMutable() const;

private:
    FramelessWidgetsHelper *q_ptr = nullptr;
    QColor m_savedWindowBackgroundColor = {};
    bool m_blurBehindWindowEnabled = false;
    QPointer<QWidget> m_window = nullptr;
    bool m_destroying = false;
    bool m_qpaReady = false;
    QSizePolicy m_savedSizePolicy = {};
    quint32 m_qpaWaitTime = 0;
};

FRAMELESSHELPER_END_NAMESPACE
