QT -= core gui

TARGET = core
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++14
CONFIG(debug, release|debug):DEFINES += _DEBUG

#QMAKE_CFLAGS_RELEASE += -Zi
#QMAKE_CXXFLAGS_RELEASE += -Zi
#QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF

SOURCES += \
	$$PWD/../../thirdparty/include/miniz/miniz.cpp

HEADERS +=

include(baking/baking.pri)
include(exporting/exporting.pri)
include(file/file.pri)
include(gzip/gzip.pri)
include(image/image.pri)
include(logging/logging.pri)
include(progress/progress.pri)
include(rendering/rendering.pri)
include(resource/resource.pri)
include(sampler/sampler.pri)
include(scene/scene.pri)
include(take/take.pri)
include(testing/testing.pri)

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
