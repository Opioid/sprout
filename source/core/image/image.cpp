#include "image.hpp"
#include "base/math/vector.inl"

namespace image {

Image::Description::Description() {}

Image::Description::Description(Type type, math::uint2 dimensions) : type(type), dimensions(dimensions) {}

Image::Image(const Description& description) : description_(description) {}

Image::~Image() {}

const Image::Description& Image::description() const {
	return description_;
}

}
