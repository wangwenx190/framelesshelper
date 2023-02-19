TEMPLATE = app
TARGET = FramelessHelperDemo-Quick
QT += qml quick quickcontrols2
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
include(../../qmake/core.pri)
include(../../qmake/quick.pri)
