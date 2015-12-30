SOURCES += \
	scene/shape/canopy.cpp \
	scene/shape/celestial_disk.cpp \
	scene/shape/disk.cpp \
	scene/shape/inverse_sphere.cpp \
	scene/shape/plane.cpp \
	scene/shape/shape.cpp \
	scene/shape/sphere.cpp \

HEADERS += \
	scene/shape/canopy.hpp \
	scene/shape/celestial_disk.hpp \
	scene/shape/disk.hpp \
	scene/shape/inverse_sphere.hpp \
	scene/shape/morphable_shape.hpp \
	scene/shape/node_stack.hpp \
	scene/shape/node_stack.inl \
	scene/shape/plane.hpp \
	scene/shape/shape.hpp \
	scene/shape/shape_sample.hpp \
	scene/shape/sphere.hpp \

include(geometry/geometry.pri)
include(triangle/triangle.pri)
