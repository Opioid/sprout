SOURCES += \
	image/texture/texture.cpp \
	image/texture/texture_byte_1_unorm.cpp \
	image/texture/texture_byte_2_unorm.cpp \
	image/texture/texture_byte_2_snorm.cpp \
	image/texture/texture_byte_3_snorm.cpp \
	image/texture/texture_byte_3_srgb.cpp \
	image/texture/texture_byte_3_unorm.cpp \
	image/texture/texture_float_1.cpp \
	image/texture/texture_float_3.cpp \
	image/texture/texture_provider.cpp \
	image/texture/texture_encoding.cpp \
	image/texture/texture_test.cpp \

HEADERS += \
	image/texture/texture_adapter.hpp \
	image/texture/texture_adapter.inl \
	image/texture/texture.hpp \
	image/texture/texture_byte_1_unorm.hpp \	
	image/texture/texture_byte_2_unorm.hpp \	
	image/texture/texture_byte_2_snorm.hpp \	
	image/texture/texture_byte_3_snorm.hpp \
	image/texture/texture_byte_3_srgb.hpp \
	image/texture/texture_byte_3_unorm.hpp \	
	image/texture/texture_float_1.hpp \
	image/texture/texture_float_3.hpp \
	image/texture/texture_provider.hpp \
	image/texture/texture_encoding.hpp \
	image/texture/texture_test.hpp \
	image/texture/texture_types.hpp \

include(sampler/sampler.pri)
