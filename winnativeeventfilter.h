#pragma once

#include <QAbstractNativeEventFilter>
#include <QRect>
#include <QVector>
#include <qt_windows.h>

class WinNativeEventFilter : public QAbstractNativeEventFilter {
    Q_DISABLE_COPY_MOVE(WinNativeEventFilter)

public:
    using WINDOWDATA = struct _WINDOWDATA {
        BOOL blurEnabled = FALSE;
        int borderWidth = -1, borderHeight = -1, titlebarHeight = -1;
        QVector<QRect> ignoreAreas;
    };
    typedef struct tagWINDOW {
        HWND hWnd = nullptr;
        BOOL dwmCompositionEnabled = FALSE, themeEnabled = FALSE,
             inited = FALSE;
        WINDOWDATA windowData;
    } WINDOW, *LPWINDOW;

    explicit WinNativeEventFilter();
    ~WinNativeEventFilter() override;

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

    static void setWindowData(HWND window, WINDOWDATA *data);
    static WINDOWDATA *windowData(HWND window);

    // Dots-Per-Inch of the given window.
    UINT windowDpi(HWND handle) const;
    // Device-Pixel-Ratio of the given window.
    qreal windowDpr(HWND handle) const;

    // DPI-aware border width of the given window.
    int borderWidth(HWND handle) const;
    // DPI-aware border height of the given window.
    int borderHeight(HWND handle) const;
    // DPI-aware titlebar height of the given window.
    int titlebarHeight(HWND handle) const;

    // Let the given window redraw itself.
    static void refreshWindow(HWND handle);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    bool nativeEventFilter(const QByteArray &eventType, void *message,
                           qintptr *result) override;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message,
                           long *result) override;
#endif

private:
    void init(LPWINDOW data);
    void handleDwmCompositionChanged(LPWINDOW data);
    void handleThemeChanged(LPWINDOW data);
    void handleBlurForWindow(LPWINDOW data);
    UINT getDpiForWindow(HWND handle) const;
    qreal getDprForWindow(HWND handle) const;
    int getSystemMetricsForWindow(HWND handle, int index) const;

private:
    using WINDOWCOMPOSITIONATTRIB = enum _WINDOWCOMPOSITIONATTRIB {
        WCA_ACCENT_POLICY = 19
    };
    using WINDOWCOMPOSITIONATTRIBDATA = struct _WINDOWCOMPOSITIONATTRIBDATA {
        DWORD dwAttribute;
        PVOID pvAttribute;
        DWORD cbAttribute;
    };
    using ACCENT_STATE = enum _ACCENT_STATE {
        ACCENT_DISABLED = 0,
        ACCENT_ENABLE_GRADIENT = 1,
        ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
        ACCENT_ENABLE_BLURBEHIND = 3,
        ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
        ACCENT_INVALID_STATE = 5
    };
    using ACCENT_POLICY = struct _ACCENT_POLICY {
        ACCENT_STATE AccentState;
        DWORD AccentFlags;
        DWORD GradientColor;
        DWORD AnimationId;
    };
    using MONITOR_DPI_TYPE = enum _MONITOR_DPI_TYPE {
        MDT_EFFECTIVE_DPI = 0,
        MDT_ANGULAR_DPI = 1,
        MDT_RAW_DPI = 2,
        MDT_DEFAULT = MDT_EFFECTIVE_DPI
    };

    const UINT m_defaultDPI = 96;
    const qreal m_defaultDPR = 1.0;

    using lpGetSystemDpiForProcess = UINT(WINAPI *)(HANDLE);
    lpGetSystemDpiForProcess m_GetSystemDpiForProcess = nullptr;
    using lpGetDpiForWindow = UINT(WINAPI *)(HWND);
    lpGetDpiForWindow m_GetDpiForWindow = nullptr;
    using lpGetDpiForSystem = UINT(WINAPI *)();
    lpGetDpiForSystem m_GetDpiForSystem = nullptr;
    using lpGetSystemMetricsForDpi = int(WINAPI *)(int, UINT);
    lpGetSystemMetricsForDpi m_GetSystemMetricsForDpi = nullptr;
    using lpGetDpiForMonitor = HRESULT(WINAPI *)(HMONITOR, MONITOR_DPI_TYPE,
                                                 UINT *, UINT *);
    lpGetDpiForMonitor m_GetDpiForMonitor = nullptr;
    using lpSetWindowCompositionAttribute =
        BOOL(WINAPI *)(HWND, WINDOWCOMPOSITIONATTRIBDATA *);
    lpSetWindowCompositionAttribute m_SetWindowCompositionAttribute = nullptr;
};
