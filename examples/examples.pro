TEMPLATE = subdirs
CONFIG -= ordered
qtHaveModule(widgets): SUBDIRS += widget
qtHaveModule(quick): SUBDIRS += quick
