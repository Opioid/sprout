SOURCES += \
	rendering/integrator/surface/ao.cpp \
	rendering/integrator/surface/normal.cpp \
	rendering/integrator/surface/pathtracer.cpp \
	rendering/integrator/surface/pathtracer_dl.cpp \
	rendering/integrator/surface/pathtracer_mis.cpp \
	rendering/integrator/surface/pathtracer_mis2.cpp \
	rendering/integrator/surface/surface_integrator.cpp \
	rendering/integrator/surface/whitted.cpp \

HEADERS += \
	rendering/integrator/surface/ao.hpp \
	rendering/integrator/surface/integrator_helper.hpp \
	rendering/integrator/surface/normal.hpp \
	rendering/integrator/surface/pathtracer.hpp \
	rendering/integrator/surface/pathtracer_dl.hpp \
	rendering/integrator/surface/pathtracer_mis.hpp \
	rendering/integrator/surface/pathtracer_mis2.hpp \
	rendering/integrator/surface/surface_integrator.hpp \
	rendering/integrator/surface/whitted.hpp \

include(transmittance/transmittance.pri)