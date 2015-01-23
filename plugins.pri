win32 {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    !contains(QMAKE_TARGET.arch, x86_64) {
        #message("win32 build")
        ARCH = Win32## Windows x86 (32bit) specific build here
    } else {
        #message("x64 build")
        ARCH = x64 ## Windows x64 (64bit) specific build here
    }

    CONFIG(debug, debug|release): BUILD = debug

    CONFIG(release, debug|release){
        BUILD=release
        #DEFINES += QT_NO_DEBUG_OUTPUT
        #DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT QT_NO_DEBUG_STREAM
        QMAKE_CFLAGS_RELEASE += -zi
        QMAKE_LFLAGS_RELEASE += /DEBUG
    }
}

win32-g++:{
#add c++ support for the gcc complier.
    CONFIG += c++11
}

DESTDIR = $$PWD/../../plugins/$$ARCH/$$BUILD/$$TARGET
OBJECTS_DIR = $$PWD/../../build/$$ARCH/$$BUILD/OAPlugins_OBJ/$$TARGET
MOC_DIR = $$PWD/../../build/$$ARCH/$$BUILD/OAPlugins_OBJ/$$TARGET
RCC_DIR = $$PWD/../../build/$$ARCH/$$BUILD/OAPlugins_OBJ/$$TARGET
UI_DIR = $$PWD/../../build/$$ARCH/$$BUILD/OAPlugins_OBJ/$$TARGET

#message($$DESTDIR  $$TARGET)
INCLUDEPATH    += $$PWD/interfaces

TEMPLATE = lib
CONFIG += plugin

# install
target.path = $$DESTDIR
INSTALLS += target

message(installs: $$DESTDIR)
