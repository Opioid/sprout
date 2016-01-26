SOURCES += \
	rendering/rendering_driver.cpp \
	rendering/rendering_worker.cpp \
	rendering/tile_queue.cpp \

HEADERS += \
	rendering/rendering_driver.hpp \
	rendering/rendering_worker.hpp \
	rendering/tile_queue.hpp \

include(integrator/integrator.pri)
include(sensor/sensor.pri)