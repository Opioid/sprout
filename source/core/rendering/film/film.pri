SOURCES += \
	rendering/film/film.cpp \
	rendering/film/unfiltered.cpp \
	
HEADERS += \
	rendering/film/film.hpp \
	rendering/film/filtered.hpp \
	rendering/film/filtered.inl \
	rendering/film/unfiltered.hpp \

include(filter/filter.pri)
include(tonemapping/tonemapping.pri)
