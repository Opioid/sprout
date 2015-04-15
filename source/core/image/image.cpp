#include "image.hpp"
#include "base/math/vector.inl"

namespace image {

Description::Description() {}

Description::Description(const math::uint2& dimensions) : dimensions(dimensions) {}

Image::Image(const Description& description) : description_(description) {}

Image::~Image() {}

const Description& Image::description() const {
	return description_;
}

}
