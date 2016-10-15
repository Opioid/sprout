SOURCES += \
	rendering/postprocessor/postprocessor.cpp \
	rendering/postprocessor/postprocessor_pipeline.cpp \
    rendering/postprocessor/postprocessor_bloom.cpp \

HEADERS += \
    rendering/postprocessor/postprocessor.hpp \
	rendering/postprocessor/postprocessor_pipeline.hpp \
    rendering/postprocessor/postprocessor_bloom.hpp \

include(tonemapping/tonemapping.pri)