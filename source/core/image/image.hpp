#ifndef SU_CORE_IMAGE_IMAGE_HPP
#define SU_CORE_IMAGE_IMAGE_HPP

#include "base/math/vector3.hpp"
#include <cstddef>

namespace image {

class Image {

public:

	enum class Type {
		Undefined,
		Byte1,
		Byte2,
		Byte3,
		Float1,
		Float2,
		Float3,
		Float4
	};

	struct Description {
		Description() = default;
		Description(Type type, int2 dimensions, int32_t num_elements = 1);
		Description(Type type, int3 const& dimensions, int32_t num_elements = 1);

		size_t num_pixels() const;

		uint32_t num_channels() const;

		Type type = Type::Undefined;
		int3 dimensions = int3(0, 0, 0);

		int32_t num_elements = 0;
	};

	Image() = default;
	Image(Description const& description);
	virtual ~Image();

	Description const& description() const;

	int2 dimensions2() const;
	int32_t area() const;
	int32_t volume() const;

	int2 coordinates_2(int32_t index) const;

	virtual size_t num_bytes() const = 0;

protected:

	void resize(Description const& description);

	Description description_;

	int32_t area_ = 0;
	int32_t volume_ = 0;
};

}

#endif
