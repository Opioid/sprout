win32:QMAKE_CXXFLAGS_WARN_ON -= -W3
win32:QMAKE_CXXFLAGS_WARN_ON += -W4
win32:QMAKE_CXXFLAGS += /wd4201 #disable warning: nameless struct/union extension
win32:QMAKE_CXXFLAGS += /wd4324 #disable warning: pad due to alignment specifier
win32:QMAKE_CXXFLAGS += /wd4505 #disable warning: unreferenced local function has been removed
