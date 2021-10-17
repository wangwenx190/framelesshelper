#pragma once

#include <QtWidgets/qwidget.h>

#include "core/framelesshelper.h"

FRAMELESSHELPER_BEGIN_NAMESPACE

template <typename T>
class FramelessWindow : public T
{
    static_assert(std::is_base_of<QWidget, T>::value, "T must inherit from QWidget");

public:
    explicit FramelessWindow(QWidget *parent = nullptr)
        : T(parent), m_helper(new FramelessHelper) {}

    ~FramelessWindow() {
        delete m_helper;
    }

    FramelessHelper *helper() { return m_helper; }

    void setResizable(bool reziable)
    {
        m_helper->setResizable(reziable);
    }

    void setHitTestVisible(QObject *obj)
    {
        m_helper->setHitTestVisible(obj);
    }

    void setResizeBorderThickness(int thickness) {
        m_helper->setResizeBorderThickness(thickness);
    }

    void setTitleBarHeight(int height)
    {
        m_helper->setTitleBarHeight(height);
    }

protected:
    void showEvent(QShowEvent *event) override
    {
        T::showEvent(event);

        if (!m_initied) {
            const auto win = this->windowHandle();
            if (win) {
                m_helper->setWindow(win);
                m_helper->install();
                m_initied = true;
            }
        }
    }

#ifdef Q_OS_WIN
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override
    {
        if (!m_helper)
            return T::nativeEvent(eventType, message, result);

        if (m_helper->handleNativeEvent(this->windowHandle(), eventType, message, result))
            return true;
        else
            return T::nativeEvent(eventType, message, result);
    }
#endif // Q_OS_WIN

private:
    FramelessHelper *m_helper;
    bool m_initied = false;
};

FRAMELESSHELPER_END_NAMESPACE
