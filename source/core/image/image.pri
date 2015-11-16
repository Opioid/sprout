SOURCES += \
	image/image.cpp \
	image/image_provider.cpp \
	image/image_writer.cpp \

HEADERS += \
	image/image.hpp \
	image/image_provider.hpp \
	image/image_writer.hpp \
	image/tiled_image.hpp \
	image/tiled_image.inl \
	image/typed_image.hpp \
	image/typed_image.inl \

include(encoding/encoding.pri)
include(texture/texture.pri)
