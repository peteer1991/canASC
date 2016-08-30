#-------------------------------------------------
#
# Project created by QtCreator 2015-11-06T23:09:03
#
#-------------------------------------------------

QT       += core gui
QT += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BLVcontroller
TEMPLATE = app


SOURCES += main.cpp\
        rotorc.cpp \
    txbar.cpp \
    can/canworkerthread.cpp \
    can/canwrapper.cpp \
    amps.cpp

HEADERS  += rotorc.h \
    txbar.h \
    can/canworkerthread.h \
    can/canwrapper.h \
    amps.h

FORMS    += rotorc.ui \
    amps.ui \
    txbar.ui

DISTFILES +=

RESOURCES += \
    resorses.qrc
