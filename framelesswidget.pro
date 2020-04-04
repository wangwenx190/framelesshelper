TARGET = framelesswidget
TEMPLATE = app
QT += gui-private widgets
CONFIG += c++17 strict_c++ utf8_source warn_on
win32 {
    CONFIG += windeployqt
    CONFIG -= embed_manifest_exe
    RC_FILE = resources.rc
    LIBS += -luser32 -lshell32 -lgdi32 -ldwmapi -luxtheme -ld2d1
    HEADERS += winnativeeventfilter.h
    SOURCES += winnativeeventfilter.cpp
}
DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII
SOURCES += main.cpp
