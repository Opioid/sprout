SOURCES += \
	image/texture/texture_2d.cpp \
	image/texture/texture_2d_byte_1_unorm.cpp \
	image/texture/texture_2d_byte_2_unorm.cpp \
	image/texture/texture_2d_byte_3_snorm.cpp \
	image/texture/texture_2d_byte_3_srgb.cpp \
	image/texture/texture_2d_byte_3_unorm.cpp \
	image/texture/texture_2d_float_3.cpp \
	image/texture/texture_2d_provider.cpp \
	image/texture/texture_encoding.cpp \

HEADERS += \
	image/texture/texture_2d.hpp \
	image/texture/texture_2d_byte_1_unorm.hpp \	
	image/texture/texture_2d_byte_2_unorm.hpp \	
	image/texture/texture_2d_byte_3_snorm.hpp \
	image/texture/texture_2d_byte_3_srgb.hpp \
	image/texture/texture_2d_byte_3_unorm.hpp \	
	image/texture/texture_2d_float_3.hpp \
	image/texture/texture_2d_provider.hpp \
	image/texture/texture_encoding.hpp \

include(sampler/sampler.pri)
