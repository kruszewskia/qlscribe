# -------------------------------------------------
# Project created by QtCreator 2009-02-22T16:31:18
# -------------------------------------------------
QT -= core \
    gui
TARGET = lscribed
TEMPLATE = lib
INCLUDEPATH += /usr/include/dbus-1.0/
CONFIG += staticlib
SOURCES += main.cpp \
    dbuscpp.cpp \
    managerhandler.cpp \
    drivehandler.cpp \
    introspecthandler.cpp
HEADERS += dbuscpp.h \
    drivehandler.h \
    managerhandler.h \
    introspecthandler.h
