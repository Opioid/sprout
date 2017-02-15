#pragma once

#include "base/math/vector.hpp"

namespace image {

class Image {

public:

	enum class Type {
		Unknown,
		Byte_1,
		Byte_2,
		Byte_3,
		Float_1,
		Float_2,
		Float_3,
		Float_4
	};

	struct Description {
		Description();
		Description(Type type, int2 dimensions, int32_t num_elements = 1);
		Description(Type type, const int3& dimensions, int32_t num_elements = 1);

		size_t num_bytes() const;

		Type type;
		int3 dimensions;

		int32_t num_elements;
	};

	Image();
	Image(const Description& description);
	virtual ~Image();

	void resize(const Description& description);

	const Description& description() const;

	int32_t area() const;
	int32_t volume() const;

	int2 coordinates_2(int32_t index) const;

	virtual size_t num_bytes() const = 0;

protected:

	Description description_;

	int32_t area_;
	int32_t volume_;
};

}
