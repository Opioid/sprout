SOURCES += \
	rendering/renderer.cpp \
	rendering/worker.cpp \

HEADERS += \
	rendering/context.hpp \
	rendering/rectangle.hpp \
	rendering/renderer.hpp \
	rendering/worker.hpp \

include(film/film.pri)
include(integrator/integrator.pri)
include(sampler/sampler.pri)
