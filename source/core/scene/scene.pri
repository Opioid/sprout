SOURCES += \
	scene/scene.cpp \
	scene/scene_loader.cpp \

HEADERS += \
	scene/scene.hpp \
	scene/scene_loader.hpp \

include(camera/camera.pri)
include(entity/entity.pri)
include(prop/prop.pri)
include(shape/shape.pri)
include(surrounding/surrounding.pri)
