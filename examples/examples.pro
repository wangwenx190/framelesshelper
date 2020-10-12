TEMPLATE = subdirs
CONFIG -= ordered
qtHaveModule(widgets): SUBDIRS += QWidget QMainWindow
qtHaveModule(quick): SUBDIRS += Quick
