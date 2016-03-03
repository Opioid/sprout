#include "image.hpp"
#include "base/math/vector.inl"

namespace image {

Image::Description::Description() {}

Image::Description::Description(Type type, math::int2 dimensions, int32_t num_elements) :
	type(type),
	dimensions(dimensions),
	num_elements(num_elements) {}

Image::Image(const Description& description) :
	description_(description),
	area_(description.dimensions.x * description.dimensions.y) {}

Image::~Image() {}

const Image::Description& Image::description() const {
	return description_;
}

}
