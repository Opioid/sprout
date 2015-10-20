#include "texture_2d_byte_3_unorm.hpp"
#include "image/image.hpp"
#include "base/color/color.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_2D_byte_3_unorm::Texture_2D_byte_3_unorm(std::shared_ptr<Image> image) :
	Texture_2D(image),
	data_(reinterpret_cast<const color::Color3c*>(image->data())) {}

float Texture_2D_byte_3_unorm::at_1(uint32_t x, uint32_t y) const {
	uint32_t i = y * dimensions().x + x;

	auto& value = data_[i];
	return color::unorm_to_float(value.x);
}

math::float2 Texture_2D_byte_3_unorm::at_2(uint32_t x, uint32_t y) const {
	uint32_t i = y * dimensions().x + x;

	auto& value = data_[i];
	return math::float2(color::unorm_to_float(value.x),
						color::unorm_to_float(value.y));
}

math::float3 Texture_2D_byte_3_unorm::at_3(uint32_t x, uint32_t y) const {
	uint32_t i = y * dimensions().x + x;

	auto& value = data_[i];
	return math::float3(color::unorm_to_float(value.x),
						color::unorm_to_float(value.y), color::unorm_to_float(value.z));
}

math::float4 Texture_2D_byte_3_unorm::at_4(uint32_t x, uint32_t y) const {
	uint32_t i = y * dimensions().x + x;

	auto& value = data_[i];
	return math::float4(color::unorm_to_float(value.x),
						color::unorm_to_float(value.y),
						color::unorm_to_float(value.z), 1.f);
}

}}
