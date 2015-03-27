SOURCES += \
	scene/shape/celestial_disk.cpp \
	scene/shape/plane.cpp \
	scene/shape/shape.cpp \
	scene/shape/sphere.cpp \

HEADERS += \
	scene/shape/celestial_disk.hpp \
	scene/shape/plane.hpp \
	scene/shape/shape.hpp \
	scene/shape/sphere.hpp \

include(geometry/geometry.pri)
include(triangle/triangle.pri)
