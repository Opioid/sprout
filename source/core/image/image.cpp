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

void Image::set_num_elements(int32_t num_elements) {
	int32_t dy = description_.num_elements * description_.dimensions.y;

	description_.dimensions.y = dy / num_elements;

	area_ = description_.dimensions.x * description_.dimensions.y;

	description_.num_elements = num_elements;
}

}
