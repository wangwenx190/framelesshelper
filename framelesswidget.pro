TARGET = framelesswidget
TEMPLATE = app
QT += widgets
CONFIG += c++17 strict_c++ utf8_source warn_on
win32 {
    CONFIG -= embed_manifest_exe
    RC_FILE = resources.rc
    LIBS += -luser32 -lshell32 -lgdi32 -ldwmapi
}
DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII
HEADERS += winnativeeventfilter.h
SOURCES += main.cpp winnativeeventfilter.cpp
