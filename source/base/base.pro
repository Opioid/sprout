QT -= core gui

TARGET = base
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++11
CONFIG(debug, release|debug):DEFINES += _DEBUG

SOURCES +=

HEADERS +=

include(color/color.pri)
include(math/math.pri)

unix {
	target.path = /usr/lib
	INSTALLS += target
}

win32:INCLUDEPATH += .
