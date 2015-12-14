#include "texture_2d_byte_1_unorm.hpp"
#include "texture_encoding.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_2D_byte_1_unorm::Texture_2D_byte_1_unorm(std::shared_ptr<Image> image) :
	Texture_2D(image),
	image_(*dynamic_cast<const Image_byte_1*>(image.get())) {}

float Texture_2D_byte_1_unorm::at_1(int32_t x, int32_t y) const {
	auto& value = image_.at(x, y);
	return encoding::unorm_to_float(value);
}

math::float2 Texture_2D_byte_1_unorm::at_2(int32_t x, int32_t y) const {
	auto& value = image_.at(x, y);
	return math::float2(encoding::unorm_to_float(value), 0.f);
}

math::float3 Texture_2D_byte_1_unorm::at_3(int32_t x, int32_t y) const {
	auto& value = image_.at(x, y);
	return math::float3(encoding::unorm_to_float(value), 0.f, 0.f);
}

math::float4 Texture_2D_byte_1_unorm::at_4(int32_t x, int32_t y) const {
	auto& value = image_.at(x, y);
	return math::float4(encoding::unorm_to_float(value), 0.f, 0.f, 1.f);
}

}}
