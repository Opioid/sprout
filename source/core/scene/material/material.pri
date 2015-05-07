SOURCES += \
	scene/material/material.cpp \
	scene/material/material_provider.cpp \

HEADERS += \
	scene/material/bxdf.hpp \
	scene/material/material.hpp \
	scene/material/material_provider.hpp \
	scene/material/material_sample.hpp \
	scene/material/material_sample_cache.hpp \
	scene/material/material_sample_cache.inl \

include(ggx/ggx.pri)
include(glass/glass.pri)
include(light/light.pri)
include(substitute/substitute.pri)
