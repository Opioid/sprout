SOURCES += \
	scene/scene.cpp \
	scene/scene_loader.cpp \
	scene/scene_worker.cpp \

HEADERS += \
	scene/scene.hpp \
	scene/scene_loader.hpp \
	scene/scene_ray.hpp \
	scene/scene_ray.inl \
	scene/scene_worker.hpp \

include(animation/animation.pri)
include(bvh/bvh.pri)
include(camera/camera.pri)
include(entity/entity.pri)
include(light/light.pri)
include(material/material.pri)
include(prop/prop.pri)
include(shape/shape.pri)
include(volume/volume.pri)
