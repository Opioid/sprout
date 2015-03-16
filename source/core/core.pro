QT -= core gui

TARGET = core
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++14
CONFIG(debug, release|debug):DEFINES += _DEBUG

SOURCES +=

HEADERS +=

include(image/image.pri)
include(rendering/rendering.pri)
include(scene/scene.pri)
include(take/take.pri)

unix {
	target.path = /usr/lib
	INSTALLS += target
}

win32:INCLUDEPATH += .

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD/..

INCLUDEPATH += $$PWD/../base
DEPENDPATH += $$PWD/../base

INCLUDEPATH += $$PWD/../../thirdparty/include
DEPENDPATH += $$PWD/../../thirdparty/include
