#include "image.hpp"
#include "base/math/vector.inl"

namespace image {

Image::Description::Description() {}

Image::Description::Description(const math::uint2& dimensions) : dimensions(dimensions) {}

Image::Image(const Description& description) : description_(description) {}

Image::~Image() {}

const Image::Description& Image::description() const {
	return description_;
}

math::float4 Image::average() const {
	const uint32_t len = description_.dimensions.x * description_.dimensions.y;

	math::float4 average = math::float4::identity;

	for (uint32_t i = 0; i < len; ++i) {
		average += at4(i);
	}

	return average;
}

}
