QT -= core gui

TARGET = base
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++14
CONFIG += rtti_off
CONFIG(debug, release|debug):DEFINES += _DEBUG

include(../compiler_options.pri)

SOURCES +=

HEADERS += \
    simd.hpp \

include(atomic/atomic.pri)
include(chrono/chrono.pri)
include(crypto/crypto.pri)
include(debug/debug.pri)
include(encoding/encoding.pri)
include(flags/flags.pri)
include(json/json.pri)
include(math/math.pri)
include(memory/memory.pri)
include(net/net.pri)
include(platform/platform.pri)
include(random/random.pri)
include(spectrum/spectrum.pri)
include(string/string.pri)
include(thread/thread.pri)

unix {
	target.path = /usr/lib
	INSTALLS += target
}

win32:INCLUDEPATH += .

INCLUDEPATH += $$PWD/../../thirdparty/include
DEPENDPATH += $$PWD/../../thirdparty/include
