#pragma once

#include "base/math/vector.hpp"

namespace image {

class Image {
public:

	enum class Type {
		Byte_1,
		Byte_2,
		Byte_3,
		Float_3,
		Float_4
	};

	struct Description {
		Description();
		Description(Type type, math::uint2 dimensions);

		Type type;
		math::uint2 dimensions;
	};

	Image(const Description& description);
	virtual ~Image();

	const Description& description() const;

	virtual const void* data() const = 0;

protected:

	Description description_;
};

}
