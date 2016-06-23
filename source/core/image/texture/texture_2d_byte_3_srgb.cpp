#include "texture_2d_byte_3_srgb.hpp"
#include "texture_encoding.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_2D_byte_3_sRGB::Texture_2D_byte_3_sRGB(std::shared_ptr<Image> image) :
	Texture_2D(image),
	image_(*dynamic_cast<const Image_byte_3*>(image.get())) {}

float Texture_2D_byte_3_sRGB::at_1(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return encoding::srgb_to_float(value.x);
}

float2 Texture_2D_byte_3_sRGB::at_2(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float2(encoding::srgb_to_float(value.x),
						encoding::srgb_to_float(value.y));
}

float3 Texture_2D_byte_3_sRGB::at_3(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float3(encoding::srgb_to_float(value.x),
						encoding::srgb_to_float(value.y),
						encoding::srgb_to_float(value.z));
}

float4 Texture_2D_byte_3_sRGB::at_4(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float4(encoding::srgb_to_float(value.x),
						encoding::srgb_to_float(value.y),
						encoding::srgb_to_float(value.z), 1.f);
}

float Texture_2D_byte_3_sRGB::at_1(int32_t x, int32_t y, int32_t element) const {
	auto& value = image_.at(x, y, element);
	return encoding::srgb_to_float(value.x);
}

float2 Texture_2D_byte_3_sRGB::at_2(int32_t x, int32_t y, int32_t element) const {
	auto& value = image_.at(x, y, element);
	return float2(encoding::srgb_to_float(value.x),
						encoding::srgb_to_float(value.y));
}

float3 Texture_2D_byte_3_sRGB::at_3(int32_t x, int32_t y, int32_t element) const {
	auto& value = image_.at(x, y, element);
	return float3(encoding::srgb_to_float(value.x),
						encoding::srgb_to_float(value.y),
						encoding::srgb_to_float(value.z));
}

float4 Texture_2D_byte_3_sRGB::at_4(int32_t x, int32_t y, int32_t element) const {
	auto& value = image_.at(x, y, element);
	return float4(encoding::srgb_to_float(value.x),
						encoding::srgb_to_float(value.y),
						encoding::srgb_to_float(value.z), 1.f);
}

}}
