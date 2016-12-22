#-------------------------------------------------
#
# Project created by QtCreator 2016-12-09T22:25:18
#
#-------------------------------------------------

QT       += core gui serialport websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG   -= app_bundle

TARGET = configtool
TEMPLATE = app



SOURCES += main.cpp\
        controll.cpp \
    server.cpp

HEADERS  += controll.h \
    server.h

FORMS    += controll.ui
