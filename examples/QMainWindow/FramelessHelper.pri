QT += gui-private

HEADERS += $$PWD/../../framelesshelper_global.h \
           $$PWD/../../winnativeeventfilter.h
           
SOURCES += $$PWD/../../winnativeeventfilter.cpp

FORMS += $$PWD/TitleBar.ui

DEFINES += FRAMELESSHELPER_STATIC \
           WNEF_WIN10_HAS_WINDOW_FRAME

RESOURCES += $$PWD/../../framelesshelper_windows.qrc