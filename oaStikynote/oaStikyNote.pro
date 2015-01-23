#-------------------------------------------------
#
# Project created by QtCreator 2015-01-19T11:49:25
#
#-------------------------------------------------
TARGET = oaStikyNote
include(../plugins.pri)
QT       += widgets network
QT       -= gui

win32-msvc*:{
    DEFINES *= W7API
    LIBS += User32.lib Ole32.lib Shell32.lib ShlWapi.lib Gdi32.lib ComCtl32.lib
}

EXAMPLE_FILES = oastikynoteplugin.json

DEFINES += OASTIKYNOTE_LIBRARY

SOURCES += oastikynote.cpp \
    formstikynote.cpp \
    networkhelper.cpp

HEADERS += oastikynote.h\
        oastikynote_global.h \
    formstikynote.h \
    networkhelper.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

TRANSLATIONS +=\
    $$_PRO_FILE_PWD_/translations/oaStikynote_zh_CN.ts\
    $$_PRO_FILE_PWD_/translations/oaStikynote_zh_TW.ts\

FORMS += \
    formstikynote.ui

RESOURCES += \
    oastikynote.qrc
