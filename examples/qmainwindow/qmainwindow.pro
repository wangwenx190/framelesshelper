TARGET = QMainWindow
TEMPLATE = app
QT += widgets gui-private
SOURCES += main.cpp
FORMS += TitleBar.ui MainWindow.ui
include($$PWD/../common.pri)
