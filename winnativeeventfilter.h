#pragma once

#include <QAbstractNativeEventFilter>
#include <QRect>
#include <QVector>
#include <qt_windows.h>

QT_BEGIN_NAMESPACE
QT_FORWARD_DECLARE_CLASS(QWindow)
QT_END_NAMESPACE

class WinNativeEventFilter : public QAbstractNativeEventFilter {
    Q_DISABLE_COPY_MOVE(WinNativeEventFilter)

public:
    using WINDOWDATA = struct _WINDOWDATA {
        BOOL blurEnabled = FALSE;
        int borderWidth = -1, borderHeight = -1, titlebarHeight = -1;
        QVector<QRect> ignoreAreas, draggableAreas;
        QSize minimumSize = {-1, -1};
    };
    typedef struct tagWINDOW {
        HWND hWnd = nullptr;
        BOOL dwmCompositionEnabled = FALSE, themeEnabled = FALSE,
             inited = FALSE;
        WINDOWDATA windowData;
    } WINDOW, *LPWINDOW;

    explicit WinNativeEventFilter();
    ~WinNativeEventFilter() override;

    static void updateQtFrame(QWindow *window);

    // Make all top level windows become frameless, unconditionally.
    static void install();
    // Make all top level windows back to normal.
    static void uninstall();

    // Frameless windows handle list
    static QVector<HWND> framelessWindows();
    static void setFramelessWindows(QVector<HWND> windows);
    // Make the given window become frameless.
    static void addFramelessWindow(HWND window, WINDOWDATA *data = nullptr);
    static void removeFramelessWindow(HWND window);
    static void clearFramelessWindows();

    // Set borderWidth, borderHeight or titlebarHeight to a negative value to
    // restore default behavior.
    static void setWindowData(HWND window, WINDOWDATA *data);
    static WINDOWDATA *windowData(HWND window);

    // DPI-aware border width of the given window.
    static int borderWidth(HWND handle);
    // DPI-aware border height of the given window.
    static int borderHeight(HWND handle);
    // DPI-aware titlebar height of the given window.
    static int titlebarHeight(HWND handle);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    bool nativeEventFilter(const QByteArray &eventType, void *message,
                           qintptr *result) override;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message,
                           long *result) override;
#endif

private:
    void init(LPWINDOW data);
    void initDLLs();
    static void createUserData(HWND handle, WINDOWDATA *data = nullptr);
    void handleDwmCompositionChanged(LPWINDOW data);
    void handleThemeChanged(LPWINDOW data);
    void handleBlurForWindow(LPWINDOW data);
    void updateRegion(LPWINDOW data);
    static void updateWindow(HWND handle);
    static UINT getDpiForWindow(HWND handle);
    static qreal getDprForWindow(HWND handle);
    static int getSystemMetricsForWindow(HWND handle, int index);
};
