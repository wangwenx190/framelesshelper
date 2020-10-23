TEMPLATE = subdirs
CONFIG -= ordered
SUBDIRS += lib examples
lib.file = lib.pro
examples.depends += lib
