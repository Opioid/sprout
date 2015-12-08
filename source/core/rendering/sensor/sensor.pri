SOURCES += \
	rendering/sensor/opaque.cpp \
	rendering/sensor/sensor.cpp \
	rendering/sensor/transparent.cpp \
	
HEADERS += \
	rendering/sensor/clamp.hpp \
	rendering/sensor/clamp.inl \
	rendering/sensor/filtered.hpp \
	rendering/sensor/filtered.inl \
	rendering/sensor/opaque.hpp \
	rendering/sensor/sensor.hpp \	
	rendering/sensor/transparent.hpp \
	rendering/sensor/unfiltered.hpp \
	rendering/sensor/unfiltered.inl \

include(filter/filter.pri)
include(tonemapping/tonemapping.pri)
