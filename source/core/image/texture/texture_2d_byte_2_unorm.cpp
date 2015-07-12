#include "texture_2d_byte_2_unorm.hpp"
#include "texture_encoding.hpp"
#include "image/image.hpp"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_2D_byte_2_unorm::Texture_2D_byte_2_unorm(std::shared_ptr<Image> image) :
	Texture_2D(image),
	data_(reinterpret_cast<const color::Color2c*>(image->data())) {}

float Texture_2D_byte_2_unorm::at_1(uint32_t x, uint32_t y) const {
	uint32_t i = y * dimensions().x + x;

	auto& value = data_[i];
	return encoding::unorm_to_float(value.x);
}

math::float2 Texture_2D_byte_2_unorm::at_2(uint32_t x, uint32_t y) const {
	uint32_t i = y * dimensions().x + x;

	auto& value = data_[i];
	return math::float2(encoding::unorm_to_float(value.x), encoding::unorm_to_float(value.y));
}

math::float3 Texture_2D_byte_2_unorm::at_3(uint32_t x, uint32_t y) const {
	uint32_t i = y * dimensions().x + x;

	auto& value = data_[i];
	return math::float3(encoding::unorm_to_float(value.x), encoding::unorm_to_float(value.y), 0.f);
}

math::float4 Texture_2D_byte_2_unorm::at_4(uint32_t x, uint32_t y) const {
	uint32_t i = y * dimensions().x + x;

	auto& value = data_[i];
	return math::float4(encoding::unorm_to_float(value.x), encoding::unorm_to_float(value.y), 0.f, 1.f);
}

}}
