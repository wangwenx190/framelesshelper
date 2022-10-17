TEMPLATE = app
TARGET = Quick
QT += qml quick quickcontrols2
win32: CONFIG -= embed_manifest_exe
DEFINES += QUICK_USE_QMAKE
HEADERS += \
    ../shared/log.h \
    ../shared/settings.h \
    quicksettings.h
SOURCES += \
    ../shared/log.cpp \
    ../shared/settings.cpp \
    quicksettings.cpp \
    main.cpp
RESOURCES += resources.qrc
win32: RC_FILE = ../shared/example.rc
include(../../qmake/core.pri)
include(../../qmake/quick.pri)
