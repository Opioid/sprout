SOURCES += \
	math/math.cpp \
	math/sampling.cpp \

HEADERS += \
	math/distribution.hpp \
	math/distribution.inl \
	math/math.hpp \
	math/matrix.hpp \
	math/matrix.inl \
	math/matrix3x3.hpp \
	math/matrix3x3.inl \
	math/matrix4x4.hpp \
	math/matrix4x4.inl \
	math/plane.hpp \
	math/plane.inl \
	math/quaternion.hpp \
	math/quaternion.inl \
	math/sampling.hpp \
	math/ray.hpp \
	math/ray.inl \
	math/vector.hpp \
	math/vector.inl \
	math/vector2.hpp \
	math/vector2.inl \
	math/vector3.hpp \
	math/vector3.inl \
	math/vector4.hpp \
	math/vector4.inl \

include(bounding/bounding.pri)
include(random/random.pri)

