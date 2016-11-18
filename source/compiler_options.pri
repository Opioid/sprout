win32 {
    QMAKE_CXXFLAGS_WARN_ON -= -W3
    QMAKE_CXXFLAGS_WARN_ON += -W4
    QMAKE_CXXFLAGS += /wd4201 #disable warning: nameless struct/union extension
    QMAKE_CXXFLAGS += /wd4324 #disable warning: pad due to alignment specifier
    QMAKE_CXXFLAGS += /wd4505 #disable warning: unreferenced local function has been removed
}

#QMAKE_CFLAGS_RELEASE += -Zi
#QMAKE_CXXFLAGS_RELEASE += -Zi
#QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF
