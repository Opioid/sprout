SOURCES += \
	image/image.cpp \
	image/image_helper.cpp \
	image/image_provider.cpp \
	image/image_writer.cpp \

HEADERS += \
	image/channels.hpp \
	image/image.hpp \
	image/image_helper.hpp \
	image/image_provider.hpp \
	image/image_writer.hpp \
	image/tiled_image.hpp \
	image/tiled_image.inl \
	image/typed_image.hpp \
	image/typed_image.inl \

include(encoding/encoding.pri)
include(filter/filter.pri)
include(procedural/procedural.pri)
include(testing/testing.pri)
include(texture/texture.pri)
