TEMPLATE = app
LANGUAGE = C++
CONFIG += qt
INCLUDEPATH += ../build/src
QMAKE_CXXFLAGS += -m32
QMAKE_LFLAGS += -m32
QMAKE_LIBDIR = /usr/lib32
SOURCES += main.cpp \
    mainwindow.cpp \
    qlightroundtextitem.cpp \
    qcdview.cpp \
    qcdscene.cpp \
    qshapefactory.cpp \
    qdialogtext.cpp \
    qdialogroundtext.cpp \
    qlightscribe.cpp \
    qdialogpixmap.cpp \
    qdialogprint.cpp \
    qdialogprogress.cpp \
    qlighttextitem.cpp \
    qlightpixmapitem.cpp
QMAKE_LIBS += -llightscribe
HEADERS += mainwindow.h \
    qlightroundtextitem.h \
    qcdview.h \
    qcdscene.h \
    qshapefactory.h \
    qdialogtext.h \
    qdialogroundtext.h \
    qlightscribe.h \
    qdialogpixmap.h \
    qdialogprint.h \
    qdialogprogress.h \
    qlighttextitem.h \
    qlightpixmapitem.h
FORMS += qdialogtext.ui \
    qdialogroundtext.ui \
    qdialogpixmap.ui \
    qdialogprint.ui \
    qdialogprogress.ui
