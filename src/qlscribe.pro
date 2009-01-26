TEMPLATE = app
LANGUAGE = C++
CONFIG += qt
QMAKE_CXXFLAGS += -m32
QMAKE_LFLAGS += -m32
QMAKE_LIBDIR = /usr/lib32
SOURCES += main.cpp \
    mainwindow.cpp \
    qgraphicsroundtextitem.cpp \
    qcdview.cpp \
    qcdscene.cpp \
    qshapefactory.cpp \
    qshapecontrollers.cpp \
    qdialogtext.cpp \
    qdialogroundtext.cpp \
    qlightscribe.cpp \
    qdialogpixmap.cpp \
    qdialogprint.cpp \
    qdialogprogress.cpp
QMAKE_LIBS += -llightscribe
HEADERS += mainwindow.h \
    qgraphicsroundtextitem.h \
    qcdview.h \
    qcdscene.h \
    qshapefactory.h \
    qshapecontrollers.h \
    qdialogtext.h \
    qdialogroundtext.h \
    qlightscribe.h \
    qdialogpixmap.h \
    qdialogprint.h \
    qdialogprogress.h
FORMS += qdialogtext.ui \
    qdialogroundtext.ui \
    qdialogpixmap.ui \
    qdialogprint.ui \
    qdialogprogress.ui
