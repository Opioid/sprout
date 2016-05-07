TARGET = sprout
TEMPLATE = app
QT -= core gui
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++14
unix: CONFIG += pthread
CONFIG(debug, release|debug):DEFINES += _DEBUG

#QMAKE_CFLAGS_RELEASE += -Zi
#QMAKE_CXXFLAGS_RELEASE += -Zi
#QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF

SOURCES += \
	main.cpp

include(controller/controller.pri)
include(options/options.pri)
include(server/server.pri)

win32:INCLUDEPATH += .

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD/..

# extension ----------------------------------------

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../extension/release/ -lextension
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../extension/debug/ -lextension
else:unix: LIBS += -L$$OUT_PWD/../extension/ -lextension

INCLUDEPATH += $$PWD/../extension
DEPENDPATH += $$PWD/../extension

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../extension/release/libextension.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../extension/debug/libextension.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../extension/release/extension.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../extension/debug/extension.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../extension/libextension.a

# core ----------------------------------------

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../core/release/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../core/debug/ -lcore
else:unix: LIBS += -L$$OUT_PWD/../core/ -lcore

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/release/libcore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/debug/libcore.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/release/core.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/debug/core.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../core/libcore.a

# base ----------------------------------------

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../base/release/ -lbase
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../base/debug/ -lbase
else:unix: LIBS += -L$$OUT_PWD/../base/ -lbase

INCLUDEPATH += $$PWD/../base
DEPENDPATH += $$PWD/../base

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../base/release/libbase.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../base/debug/libbase.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../base/release/base.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../base/debug/base.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../base/libbase.a

INCLUDEPATH += $$PWD/../../thirdparty/include
DEPENDPATH += $$PWD/../../thirdparty/include

unix: LIBS += -pthread
