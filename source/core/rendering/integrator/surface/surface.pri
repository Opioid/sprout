SOURCES += \
	rendering/integrator/surface/ao.cpp \
	rendering/integrator/surface/debug.cpp \
	rendering/integrator/surface/pathtracer.cpp \
	rendering/integrator/surface/pathtracer_dl.cpp \
	rendering/integrator/surface/pathtracer_mis.cpp \
	rendering/integrator/surface/surface_integrator.cpp \
	rendering/integrator/surface/whitted.cpp \

HEADERS += \
	rendering/integrator/surface/ao.hpp \
	rendering/integrator/surface/debug.hpp \
	rendering/integrator/surface/integrator_helper.hpp \
	rendering/integrator/surface/pathtracer.hpp \
	rendering/integrator/surface/pathtracer_dl.hpp \
	rendering/integrator/surface/pathtracer_mis.hpp \
	rendering/integrator/surface/surface_integrator.hpp \
	rendering/integrator/surface/whitted.hpp \

include(sub/sub.pri)
include(transmittance/transmittance.pri)
