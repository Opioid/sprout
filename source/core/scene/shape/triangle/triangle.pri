SOURCES += \
	scene/shape/triangle/triangle_mesh.cpp \
	scene/shape/triangle/triangle_mesh_provider.cpp \

HEADERS += \
	scene/shape/triangle/triangle_intersection.hpp \
	scene/shape/triangle/triangle_mesh.hpp \
	scene/shape/triangle/triangle_mesh_provider.hpp \
	scene/shape/triangle/triangle_primitive.hpp \
	scene/shape/triangle/triangle_primitive_mt.hpp \
	scene/shape/triangle/triangle_primitive_mt.inl \
	scene/shape/triangle/triangle_primitive_yf.hpp \
	scene/shape/triangle/triangle_primitive_yf.inl \

include(bvh/bvh.pri)

