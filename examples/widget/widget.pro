TEMPLATE = app
TARGET = Widget
QT += widgets
win32: CONFIG -= embed_manifest_exe
DEFINES += WIDGET_USE_QMAKE
HEADERS += \
    ../shared/log.h \
    ../shared/settings.h \
    widget.h
SOURCES += \
    ../shared/log.cpp \
    ../shared/settings.cpp \
    widget.cpp \
    main.cpp
win32: RC_FILE = ../shared/example.rc
include(../../qmake/core.pri)
include(../../qmake/widgets.pri)
