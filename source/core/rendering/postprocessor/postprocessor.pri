SOURCES += \
	rendering/postprocessor/postprocessor.cpp \
    rendering/postprocessor/postprocessor_bloom.cpp \
    rendering/postprocessor/postprocessor_glare.cpp \
	rendering/postprocessor/postprocessor_pipeline.cpp \

HEADERS += \
    rendering/postprocessor/postprocessor.hpp \
    rendering/postprocessor/postprocessor_bloom.hpp \
    rendering/postprocessor/postprocessor_glare.hpp \
	rendering/postprocessor/postprocessor_pipeline.hpp \

include(tonemapping/tonemapping.pri)
