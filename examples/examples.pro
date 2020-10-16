TEMPLATE = subdirs
CONFIG -= ordered
qtHaveModule(widgets): SUBDIRS += QWidget QMainWindow
qtHaveModule(quick): SUBDIRS += Quick
win32: qtHaveModule(widgets): SUBDIRS += Win32Demo
