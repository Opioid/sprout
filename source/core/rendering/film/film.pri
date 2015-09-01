SOURCES += \
	rendering/film/film.cpp \
	rendering/film/opaque.cpp \
	rendering/film/transparent.cpp \
	
HEADERS += \
	rendering/film/film.hpp \
	rendering/film/filtered.hpp \
	rendering/film/filtered.inl \
	rendering/film/opaque.hpp \
	rendering/film/transparent.hpp \
	rendering/film/unfiltered.hpp \
	rendering/film/unfiltered.inl \

include(filter/filter.pri)
include(tonemapping/tonemapping.pri)
