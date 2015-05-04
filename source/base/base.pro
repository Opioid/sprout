QT -= core gui

TARGET = base
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++14
CONFIG(debug, release|debug):DEFINES += _DEBUG

SOURCES +=

HEADERS +=

include(color/color.pri)
include(file/file.pri)
include(json/json.pri)
include(math/math.pri)
include(thread/thread.pri)

unix {
	target.path = /usr/lib
	INSTALLS += target
}

win32:INCLUDEPATH += .

INCLUDEPATH += $$PWD/../../thirdparty/include
DEPENDPATH += $$PWD/../../thirdparty/include
