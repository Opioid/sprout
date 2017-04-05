SOURCES += \
	scene/shape/triangle/triangle_json_handler.cpp \
	scene/shape/triangle/triangle_mesh.cpp \
	scene/shape/triangle/triangle_mesh_exporter.cpp \
	scene/shape/triangle/triangle_mesh_provider.cpp \
	scene/shape/triangle/triangle_morph_target_collection.cpp \
	scene/shape/triangle/triangle_morphable_mesh.cpp \
	scene/shape/triangle/vertex_layout_description.cpp \

HEADERS += \
	scene/shape/triangle/triangle_bvh_preset.hpp \
	scene/shape/triangle/triangle_intersection.hpp \
	scene/shape/triangle/triangle_json_handler.hpp \
	scene/shape/triangle/triangle_mesh.hpp \
	scene/shape/triangle/triangle_mesh_exporter.hpp \
	scene/shape/triangle/triangle_mesh_generator.hpp \
	scene/shape/triangle/triangle_mesh_provider.hpp \
	scene/shape/triangle/triangle_morph_target_collection.hpp \
	scene/shape/triangle/triangle_morphable_mesh.hpp \
	scene/shape/triangle/triangle_primitive.hpp \
	scene/shape/triangle/triangle_primitive_mt.hpp \
	scene/shape/triangle/triangle_primitive_mt.inl \	
	scene/shape/triangle/triangle_type.hpp \
	scene/shape/triangle/vertex_encoding.hpp \
	scene/shape/triangle/vertex_encoding.inl \
	scene/shape/triangle/vertex_layout_description.hpp \

include(bvh/bvh.pri)
