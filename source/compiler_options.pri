win32 {
    # optimization level
#    QMAKE_CXXFLAGS_RELEASE -= -O2
#    QMAKE_CXXFLAGS_RELEASE += -Ox
    # warning level
    QMAKE_CXXFLAGS_WARN_ON -= -W3
    QMAKE_CXXFLAGS_WARN_ON += -W4
#    QMAKE_CXXFLAGS_WARN_ON += -Wall
    # disable warnings
    QMAKE_CXXFLAGS += /wd4201 #nameless struct/union extension
    QMAKE_CXXFLAGS += /wd4324 #pad due to alignment specifier
    QMAKE_CXXFLAGS += /wd4505 #unreferenced local function has been removed
    QMAKE_CXXFLAGS += /wd4514 #unreferenced inline function has been removed
    QMAKE_CXXFLAGS += /wd4582 #constructor is not implicitly called
    QMAKE_CXXFLAGS += /wd4587 #constructor is no longer implicitly called
    QMAKE_CXXFLAGS += /wd4625 #copy operator implicitly deleted
    QMAKE_CXXFLAGS += /wd4626 #assignment operator implicitly deleted
    QMAKE_CXXFLAGS += /wd4710 #function not inlined
    QMAKE_CXXFLAGS += /wd4711 #function selected for automatic inline expansion
    QMAKE_CXXFLAGS += /wd5026 #move constructor operator implicitly deleted
    QMAKE_CXXFLAGS += /wd5027 #move assignment operator implicitly deleted
} else {
#    QMAKE_CXXFLAGS += -msse4.1
}

#QMAKE_CFLAGS_RELEASE += -Zi
#QMAKE_CXXFLAGS_RELEASE += -Zi
#QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF
