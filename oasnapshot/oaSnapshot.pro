TEMPLATE     = lib
CONFIG      += plugin
win32-msvc*{
    DEFINES *= W7API
}else{
    CONFIG += c++11
}
QT          += widgets xml

INCLUDEPATH    += $$PWD/../interfaces
DESTDIR         = $$PWD/../plugins

TARGET = oaSnapshot

EXAMPLE_FILES = oaSnapshot.json

DEFINES += OASNAPSHOT_LIBRARY

SOURCES += oasnapshot.cpp \
    formsnapshot.cpp \
    oasnapdefine.cpp \
    formsnapshotsetting.cpp

HEADERS += oasnapshot.h\
        oasnapshot_global.h \
    oasnapshotinterface.h \
    ../interfaces/oasnapshotinterface.h \
    formsnapshot.h \
    oasnapdefine.h \
    oadrawcommand.h \
    oarect.h \
    formsnapshotsetting.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
TRANSLATIONS +=\
    $$_PRO_FILE_PWD_/translations/oaSnapshot_zh_CN.ts\
    $$_PRO_FILE_PWD_/translations/oaSnapshot_zh_TW.ts\

#    outputlangDir = $$_PRO_FILE_PWD_/../../../build/$$ARCH/$$BUILD/OAPlugins/translations
#updateqm.input = TRANSLATIONS
#updateqm.output = $$outputlangDir/${QMAKE_FILE_BASE}.qm
#updateqm.commands = $$QMAKE_LRELEASE -silent ${QMAKE_FILE_IN} -qm $$outputlangDir/${QMAKE_FILE_BASE}.qm
#updateqm.CONFIG += no_link target_predeps
#QMAKE_EXTRA_COMPILERS += updateqm

RESOURCES += \
    oasnapres.qrc
