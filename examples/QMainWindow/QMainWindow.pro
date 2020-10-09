TARGET = qmainwindow
CONFIG(debug, debug|release): TARGET = $$join(TARGET,,,d)
TEMPLATE = app
qtHaveModule(widgets): QT += widgets

CONFIG += c++17 strict_c++ utf8_source warn_on
DEFINES += WIN32_LEAN_AND_MEAN \
           _CRT_SECURE_NO_WARNINGS

SOURCES += main.cpp
FORMS += MainWindow.ui
!include( FramelessHelper.pri ) : error( Unable to load FramelessHelper.pri )