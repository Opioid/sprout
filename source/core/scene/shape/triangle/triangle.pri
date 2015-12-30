SOURCES += \
	scene/shape/triangle/triangle_json_handler.cpp \
	scene/shape/triangle/triangle_mesh.cpp \
	scene/shape/triangle/triangle_mesh_provider.cpp \
	scene/shape/triangle/triangle_morph_target_collection.cpp \
	scene/shape/triangle/triangle_morphable_mesh.cpp \

HEADERS += \
	scene/shape/triangle/triangle_intersection.hpp \
	scene/shape/triangle/triangle_json_handler.hpp \
	scene/shape/triangle/triangle_mesh.hpp \
	scene/shape/triangle/triangle_mesh_provider.hpp \
	scene/shape/triangle/triangle_morph_target_collection.hpp \
	scene/shape/triangle/triangle_morphable_mesh.hpp \
	scene/shape/triangle/triangle_primitive.hpp \
	scene/shape/triangle/triangle_primitive_mt.hpp \
	scene/shape/triangle/triangle_primitive_mt.inl \
	scene/shape/triangle/triangle_primitive_mte.hpp \
	scene/shape/triangle/triangle_primitive_mte.inl \	
	scene/shape/triangle/triangle_primitive_yf.hpp \
	scene/shape/triangle/triangle_primitive_yf.inl \
	scene/shape/triangle/triangle_type.hpp \

include(bvh/bvh.pri)
