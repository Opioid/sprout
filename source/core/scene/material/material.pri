SOURCES += \
	scene/material/bssrdf.cpp \
	scene/material/material.cpp \
	scene/material/material_provider.cpp \
	scene/material/material_sample_cache.cpp \
	scene/material/material_test.cpp \
	scene/material/sampler_cache.cpp \
	scene/material/sampler_settings.cpp \

HEADERS += \
	scene/material/bssrdf.hpp \
	scene/material/bxdf.hpp \
	scene/material/material.hpp \
	scene/material/material_print.hpp \
	scene/material/material_provider.hpp \
	scene/material/material_sample.hpp \
	scene/material/material_sample.inl \
	scene/material/material_sample_cache.hpp \
	scene/material/material_sample_cache.inl \
	scene/material/material_test.hpp \
	scene/material/sampler_cache.hpp \
	scene/material/sampler_settings.hpp \

include(coating/coating.pri)
include(cloth/cloth.pri)
include(disney/disney.pri)
include(display/display.pri)
include(fresnel/fresnel.pri)
include(ggx/ggx.pri)
include(glass/glass.pri)
include(lambert/lambert.pri)
include(light/light.pri)
include(matte/matte.pri)
include(metal/metal.pri)
include(metallic_paint/metallic_paint.pri)
include(oren_nayar/oren_nayar.pri)
include(sky/sky.pri)
include(substitute/substitute.pri)
