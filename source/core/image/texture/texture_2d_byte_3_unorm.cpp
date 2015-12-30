#include "texture_2d_byte_3_unorm.hpp"
#include "image/typed_image.inl"
#include "base/color/color.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_2D_byte_3_unorm::Texture_2D_byte_3_unorm(std::shared_ptr<Image> image) :
	Texture_2D(image),
	image_(*dynamic_cast<const Image_byte_3*>(image.get())) {}

float Texture_2D_byte_3_unorm::at_1(int32_t x, int32_t y) const {
	auto& value = image_.at(x, y);
	return color::unorm_to_float(value.x);
}

math::float2 Texture_2D_byte_3_unorm::at_2(int32_t x, int32_t y) const {
	auto& value = image_.at(x, y);
	return math::float2(color::unorm_to_float(value.x),
						color::unorm_to_float(value.y));
}

math::float3 Texture_2D_byte_3_unorm::at_3(int32_t x, int32_t y) const {
	auto& value = image_.at(x, y);
	return math::float3(color::unorm_to_float(value.x),
						color::unorm_to_float(value.y),
						color::unorm_to_float(value.z));
}

math::float4 Texture_2D_byte_3_unorm::at_4(int32_t x, int32_t y) const {
	auto& value = image_.at(x, y);
	return math::float4(color::unorm_to_float(value.x),
						color::unorm_to_float(value.y),
						color::unorm_to_float(value.z), 1.f);
}

}}
