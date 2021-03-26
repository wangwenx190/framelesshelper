TARGET = $$qtLibraryTarget(FramelessHelper)
TEMPLATE = lib
win32: DLLDESTDIR = $$OUT_PWD/bin
else: unix: DESTDIR = $$OUT_PWD/bin
QT += gui-private
CONFIG += c++17 strict_c++ utf8_source warn_on
DEFINES += \
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII \
    QT_NO_KEYWORDS \
    QT_DEPRECATED_WARNINGS \
    QT_DISABLE_DEPRECATED_BEFORE=0x060000 \
    FRAMELESSHELPER_BUILD_LIBRARY
HEADERS += \
    framelesshelper_global.h \
    framelesshelper.h \
    framelesswindowsmanager.h \
    utilities.h \
    qtacryliceffecthelper.h
SOURCES += \
    framelesshelper.cpp \
    framelesswindowsmanager.cpp \
    utilities.cpp \
    qtacryliceffecthelper.cpp
qtHaveModule(widgets) {
    QT += widgets
    HEADERS += qtacrylicwidget.h \
               qtacrylicmainwindow.h
    SOURCES += qtacrylicwidget.cpp \
               qtacrylicmainwindow.cpp
}
qtHaveModule(quick) {
    QT += quick
    HEADERS += \
        framelessquickhelper.h \
        qtacrylicitem.h
    SOURCES += \
        framelessquickhelper.cpp \
        qtacrylicitem.cpp
}
RESOURCES += qtacrylichelper.qrc
win32 {
    DEFINES += \
        WIN32_LEAN_AND_MEAN \
        _CRT_SECURE_NO_WARNINGS \
        UNICODE \
        _UNICODE
    HEADERS += \
        framelesshelper_win32.h \
        qtacryliceffecthelper_win32.h
    SOURCES += \
        utilities_win32.cpp \
        framelesshelper_win32.cpp \
        qtacryliceffecthelper_win32.cpp
    LIBS += -luser32 -lshell32 -lgdi32 -ldwmapi -lole32
    RC_FILE = framelesshelper.rc
}
