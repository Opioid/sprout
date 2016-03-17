QT -= core gui

TARGET = extension
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++14
CONFIG(debug, release|debug):DEFINES += _DEBUG

#QMAKE_CFLAGS_RELEASE += -Zi
#QMAKE_CXXFLAGS_RELEASE += -Zi
#QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF

include(procedural/procedural.pri)

unix {
	target.path = /usr/lib
	INSTALLS += target
}

win32:INCLUDEPATH += .

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD/..

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core
