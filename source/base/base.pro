QT -= core gui

TARGET = base
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++14
CONFIG(debug, release|debug):DEFINES += _DEBUG

#QMAKE_CFLAGS_RELEASE += -Zi
#QMAKE_CXXFLAGS_RELEASE += -Zi
#QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF

SOURCES +=

HEADERS +=

include(atomic/atomic.pri)
include(chrono/chrono.pri)
include(color/color.pri)
include(flags/flags.pri)
include(json/json.pri)
include(math/math.pri)
include(memory/memory.pri)
include(string/string.pri)
include(thread/thread.pri)

unix {
	target.path = /usr/lib
	INSTALLS += target
}

win32:INCLUDEPATH += .

INCLUDEPATH += $$PWD/../../thirdparty/include
DEPENDPATH += $$PWD/../../thirdparty/include
