QT -= core gui

TARGET = core
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++11
CONFIG(debug, release|debug):DEFINES += _DEBUG

SOURCES +=

HEADERS +=

unix {
	target.path = /usr/lib
	INSTALLS += target
}

win32:INCLUDEPATH += .
