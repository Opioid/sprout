SOURCES += \
	rendering/rendering_camera_worker.cpp \
	rendering/rendering_driver.cpp \
	rendering/rendering_driver_finalframe.cpp \
	rendering/rendering_driver_progressive.cpp \	
	rendering/rendering_worker.cpp \
	rendering/tile_queue.cpp \

HEADERS += \
	rendering/rendering_camera_worker.hpp \
	rendering/rendering_driver.hpp \
	rendering/rendering_driver_finalframe.hpp \
	rendering/rendering_driver_progressive.hpp \
	rendering/rendering_worker.hpp \
	rendering/tile_queue.hpp \

include(integrator/integrator.pri)
include(sensor/sensor.pri)