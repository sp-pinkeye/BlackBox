#-------------------------------------------------
#
# Project created by QtCreator 2015-05-22T19:01:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Blackbox
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

unix|win32: LIBS += -lrtmidi
