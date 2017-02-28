QT -= core gui

TARGET = extension
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++14
CONFIG += rtti_off
CONFIG(debug, release|debug):DEFINES += _DEBUG

include(../compiler_options.pri)

include(procedural/procedural.pri)

unix {
	target.path = /usr/lib
	INSTALLS += target
}

win32:INCLUDEPATH += .

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD/..

INCLUDEPATH += $$PWD/../base
DEPENDPATH += $$PWD/../base

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core

INCLUDEPATH += $$PWD/../../thirdparty/include
DEPENDPATH += $$PWD/../../thirdparty/include
