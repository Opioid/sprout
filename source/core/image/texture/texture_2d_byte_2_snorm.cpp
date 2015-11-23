#include "texture_2d_byte_2_snorm.hpp"
#include "texture_encoding.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_2D_byte_2_snorm::Texture_2D_byte_2_snorm(std::shared_ptr<Image> image) :
	Texture_2D(image),
	image_(*dynamic_cast<const Image_byte_2*>(image.get())) {}

float Texture_2D_byte_2_snorm::at_1(uint32_t x, uint32_t y) const {
	auto& value = image_.at(x, y);
	return encoding::snorm_to_float(value.x);
}

math::float2 Texture_2D_byte_2_snorm::at_2(uint32_t x, uint32_t y) const {
	auto& value = image_.at(x, y);
	return math::float2(encoding::snorm_to_float(value.x), encoding::snorm_to_float(value.y));
}

math::float3 Texture_2D_byte_2_snorm::at_3(uint32_t x, uint32_t y) const {
	auto& value = image_.at(x, y);
	return math::float3(encoding::snorm_to_float(value.x), encoding::snorm_to_float(value.y), 0.f);
}

math::float4 Texture_2D_byte_2_snorm::at_4(uint32_t x, uint32_t y) const {
	auto& value = image_.at(x, y);
	return math::float4(encoding::snorm_to_float(value.x), encoding::snorm_to_float(value.y), 0.f, 1.f);
}

}}
