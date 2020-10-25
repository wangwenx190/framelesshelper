TARGET = $$qtLibraryTarget(FramelessHelper)
TEMPLATE = lib
win32: DLLDESTDIR = $$OUT_PWD/bin
else: unix: DESTDIR = $$OUT_PWD/bin
QT += gui-private
qtHaveModule(widgets): QT += widgets
qtHaveModule(quick): QT += quick
CONFIG += c++17 strict_c++ utf8_source warn_on
DEFINES += \
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII \
    FRAMELESSHELPER_BUILD_LIBRARY
HEADERS += \
    framelesshelper_global.h \
    framelesswindowsmanager.h
SOURCES += framelesswindowsmanager.cpp
qtHaveModule(quick) {
    HEADERS += framelessquickhelper.h
    SOURCES += framelessquickhelper.cpp
}
win32 {
    DEFINES += WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS
    CONFIG += LINK_TO_SYSTEM_DLL
    HEADERS += winnativeeventfilter.h
    SOURCES += winnativeeventfilter.cpp
    LINK_TO_SYSTEM_DLL {
        DEFINES += WNEF_LINK_SYSLIB
        LIBS += -luser32 -lshell32 -lgdi32 -ldwmapi -lshcore -ld2d1 -luxtheme
    }
    RC_FILE = framelesshelper.rc
} else {
    HEADERS += framelesshelper.h
    SOURCES += framelesshelper.cpp
}
