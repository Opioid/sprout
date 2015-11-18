SOURCES += \
	rendering/rendering_driver.cpp \
	rendering/rendering_worker.cpp \

HEADERS += \
	rendering/rendering_context.hpp \
	rendering/rectangle.hpp \
	rendering/rendering_driver.hpp \
	rendering/rendering_worker.hpp \

include(film/film.pri)
include(integrator/integrator.pri)
