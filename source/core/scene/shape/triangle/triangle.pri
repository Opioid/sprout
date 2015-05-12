SOURCES += \
	scene/shape/triangle/triangle_mesh.cpp \
	scene/shape/triangle/triangle_mesh_provider.cpp \

HEADERS += \
	scene/shape/triangle/triangle_distribution.hpp \
	scene/shape/triangle/triangle_distribution.inl \
	scene/shape/triangle/triangle_intersection.hpp \
	scene/shape/triangle/triangle_mesh.hpp \
	scene/shape/triangle/triangle_mesh_provider.hpp \
	scene/shape/triangle/triangle_primitive.hpp \
	scene/shape/triangle/triangle_primitive.inl \

include(bvh/bvh.pri)

