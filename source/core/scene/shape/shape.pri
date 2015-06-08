SOURCES += \
	scene/shape/celestial_disk.cpp \
	scene/shape/disk.cpp \
	scene/shape/plane.cpp \
	scene/shape/shape.cpp \
	scene/shape/sphere.cpp \

HEADERS += \
	scene/shape/celestial_disk.hpp \
	scene/shape/disk.hpp \
	scene/shape/node_stack.hpp \
	scene/shape/plane.hpp \
	scene/shape/shape.hpp \
	scene/shape/shape_sample.hpp \
	scene/shape/sphere.hpp \

include(geometry/geometry.pri)
include(triangle/triangle.pri)
