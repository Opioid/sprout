#include "texture_2d.hpp"
#include "image/image.hpp"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_2D::Texture_2D(std::shared_ptr<Image> image) : Texture(image) {}

math::uint2 Texture_2D::dimensions() const {
	return image_->description().dimensions;
}

float Texture_2D::at1(uint32_t x, uint32_t y) const {
	uint32_t i = y * image_->description().dimensions.x + x;
	return image_->at1(i);
}

math::float2 Texture_2D::at2(uint32_t x, uint32_t y) const {
	uint32_t i = y * image_->description().dimensions.x + x;
	return image_->at2(i);
}

math::float3 Texture_2D::at3(uint32_t x, uint32_t y) const {
	uint32_t i = y * image_->description().dimensions.x + x;
	return image_->at3(i);
}

math::float4 Texture_2D::at4(uint32_t x, uint32_t y) const {
	uint32_t i = y * image_->description().dimensions.x + x;
	return image_->at4(i);
}

math::float4 Texture_2D::average() const {
	auto d = image_->description().dimensions;
	const uint32_t len = d.x * d.y;

	math::float4 average = math::float4::identity;

	for (uint32_t i = 0; i < len; ++i) {
		average += image_->at4(i);
	}

	return average;
}

}}
