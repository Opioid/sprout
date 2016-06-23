#include "texture_2d_byte_2_unorm.hpp"
#include "texture_encoding.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_2D_byte_2_unorm::Texture_2D_byte_2_unorm(std::shared_ptr<Image> image) :
	Texture_2D(image),
	image_(*dynamic_cast<const Image_byte_2*>(image.get())) {}

float Texture_2D_byte_2_unorm::at_1(int32_t x, int32_t y) const {
	auto& value = image_.at(x, y);
	return encoding::unorm_to_float(value.x);
}

float2 Texture_2D_byte_2_unorm::at_2(int32_t x, int32_t y) const {
	auto& value = image_.at(x, y);
	return float2(encoding::unorm_to_float(value.x), encoding::unorm_to_float(value.y));
}

float3 Texture_2D_byte_2_unorm::at_3(int32_t x, int32_t y) const {
	auto& value = image_.at(x, y);
	return float3(encoding::unorm_to_float(value.x), encoding::unorm_to_float(value.y), 0.f);
}

float4 Texture_2D_byte_2_unorm::at_4(int32_t x, int32_t y) const {
	auto& value = image_.at(x, y);
	return float4(encoding::unorm_to_float(value.x), encoding::unorm_to_float(value.y), 0.f, 1.f);
}

float Texture_2D_byte_2_unorm::at_1(int32_t x, int32_t y, int32_t element) const {
	auto& value = image_.at(x, y, element);
	return encoding::unorm_to_float(value.x);
}

float2 Texture_2D_byte_2_unorm::at_2(int32_t x, int32_t y, int32_t element) const {
	auto& value = image_.at(x, y, element);
	return float2(encoding::unorm_to_float(value.x), encoding::unorm_to_float(value.y));
}

float3 Texture_2D_byte_2_unorm::at_3(int32_t x, int32_t y, int32_t element) const {
	auto& value = image_.at(x, y, element);
	return float3(encoding::unorm_to_float(value.x), encoding::unorm_to_float(value.y), 0.f);
}

float4 Texture_2D_byte_2_unorm::at_4(int32_t x, int32_t y, int32_t element) const {
	auto& value = image_.at(x, y, element);
	return float4(encoding::unorm_to_float(value.x), encoding::unorm_to_float(value.y), 0.f, 1.f);
}

}}
