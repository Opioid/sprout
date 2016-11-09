SOURCES += \
	sampler/sampler_ems.cpp \
	sampler/sampler_golden_ratio.cpp \
	sampler/sampler_halton.cpp \
	sampler/sampler_ld.cpp \
	sampler/sampler_random.cpp \
	sampler/sampler.cpp \
	sampler/sampler_scrambled_hammersley.cpp \
    sampler/sampler_sobol.cpp \
	sampler/sampler_test.cpp \
	sampler/sampler_uniform.cpp \	

HEADERS += \
	sampler/camera_sample.hpp \
	sampler/sampler_ems.hpp \
	sampler/sampler_golden_ratio.hpp \
	sampler/sampler_halton.hpp \
	sampler/sampler_ld.hpp \
	sampler/sampler_random.hpp \
	sampler/sampler.hpp \
	sampler/sampler_scrambled_hammersley.hpp \
    sampler/sampler_sobol.hpp \
	sampler/sampler_test.hpp \
	sampler/sampler_uniform.hpp \

include(halton/halton.pri)
include(sobol/sobol.pri)

