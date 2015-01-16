#-------------------------------------------------
#
# Project created by QtCreator 2015-01-16T17:54:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH    += $$PWD/../interfaces

CONFIG  += c++11
TARGET = example
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h

FORMS    += widget.ui
