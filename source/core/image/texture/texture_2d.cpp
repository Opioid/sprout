#include "texture_2d.hpp"
#include "image/image.hpp"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_2D::Texture_2D(std::shared_ptr<Image> image) : image_(image) {}

Texture_2D::~Texture_2D() {}

math::uint2 Texture_2D::dimensions() const {
	return image_->description().dimensions;
}

math::float4 Texture_2D::average() const {
	auto d = dimensions();

	math::float4 average = math::float4::identity;

	for (uint32_t y = 0; y < d.y; ++y) {
		for (uint32_t x = 0; x < d.x; ++x) {
			average += at_4(x, y);
		}
	}

	return average;
}

}}
