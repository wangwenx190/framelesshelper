QT += core core-private gui gui-private

CORE_PUB_INC_DIR = $$PWD/../include/FramelessHelper/Core
CORE_PRIV_INC_DIR = $$CORE_PUB_INC_DIR/private
CORE_EXTRA_INC_DIR = $$PWD/inc/core
CORE_SRC_DIR = $$PWD/../src/core

DEFINES += \
    FRAMELESSHELPER_CORE_STATIC

INCLUDEPATH += \
    $$CORE_PUB_INC_DIR \
    $$CORE_PRIV_INC_DIR \
    $$CORE_EXTRA_INC_DIR

DEPENDPATH += \
    $$CORE_PUB_INC_DIR \
    $$CORE_PRIV_INC_DIR \
    $$CORE_EXTRA_INC_DIR

HEADERS += \
    $$CORE_EXTRA_INC_DIR/framelesshelper.version \
    $$CORE_PUB_INC_DIR/chromepalette.h \
    $$CORE_PUB_INC_DIR/framelesshelper_qt.h \
    $$CORE_PUB_INC_DIR/framelesshelpercore_global.h \
    $$CORE_PUB_INC_DIR/framelessmanager.h \
    $$CORE_PUB_INC_DIR/micamaterial.h \
    $$CORE_PUB_INC_DIR/utils.h \
    $$CORE_PUB_INC_DIR/windowborderpainter.h \
    $$CORE_PRIV_INC_DIR/chromepalette_p.h \
    $$CORE_PRIV_INC_DIR/framelessconfig_p.h \
    $$CORE_PRIV_INC_DIR/framelessmanager_p.h \
    $$CORE_PRIV_INC_DIR/micamaterial_p.h \
    $$CORE_PRIV_INC_DIR/sysapiloader_p.h \
    $$CORE_PRIV_INC_DIR/windowborderpainter_p.h

SOURCES += \
    $$CORE_SRC_DIR/chromepalette.cpp \
    $$CORE_SRC_DIR/framelessconfig.cpp \
    $$CORE_SRC_DIR/framelesshelper_qt.cpp \
    $$CORE_SRC_DIR/framelessmanager.cpp \
    $$CORE_SRC_DIR/framelesshelpercore_global.cpp \
    $$CORE_SRC_DIR/micamaterial.cpp \
    $$CORE_SRC_DIR/sysapiloader.cpp \
    $$CORE_SRC_DIR/utils.cpp \
    $$CORE_SRC_DIR/windowborderpainter.cpp

RESOURCES += \
    $$CORE_SRC_DIR/framelesshelpercore.qrc

win32 {
    HEADERS += \
        $$CORE_PUB_INC_DIR/framelesshelper_win.h \
        $$CORE_PUB_INC_DIR/framelesshelper_windows.h \
        $$CORE_PRIV_INC_DIR/registrykey_p.h \
        $$CORE_PRIV_INC_DIR/winverhelper_p.h
    SOURCES += \
        $$CORE_SRC_DIR/framelesshelper_win.cpp \
        $$CORE_SRC_DIR/utils_win.cpp \
        $$CORE_SRC_DIR/registrykey.cpp \
        $$CORE_SRC_DIR/winverhelper.cpp
    LIBS += -luser32 -lgdi32 -lshell32
}

unix:!macx {
    CONFIG += link_pkgconfig
    PKGCONFIG += gtk+-3.0 xcb
    DEFINES += GDK_VERSION_MIN_REQUIRED=GDK_VERSION_3_6
    SOURCES += $$CORE_SRC_DIR/utils_linux.cpp
}

macx {
    SOURCES += $$CORE_SRC_DIR/utils_mac.mm
    #LIBS += "-framework AppKit" "-framework Cocoa" "-framework Foundation" # TODO
}
