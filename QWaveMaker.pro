#-------------------------------------------------
#
# Project created by QtCreator 2014-02-06T18:05:21
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QWaveMaker
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    wavewidget.cpp \
    wavedevice.cpp

HEADERS  += widget.h \
    wavewidget.h \
    wavedevice.h

FORMS    += widget.ui

win32: {
    # QMAKE_LFLAGS += -static -static-libgcc
    LIBS += -L$$PWD/fftw -lfftw3-3
    INCLUDEPATH += $$PWD/fftw
    RC_FILE = app.rc
}

unix: LIBS += -lfftw3
