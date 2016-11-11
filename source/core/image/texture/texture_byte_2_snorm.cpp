#include "texture_byte_2_snorm.hpp"
#include "texture_encoding.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_byte_2_snorm::Texture_byte_2_snorm(std::shared_ptr<Image> image) :
	Texture(image),
	image_(*dynamic_cast<const Byte_2*>(image.get())) {}

float Texture_byte_2_snorm::at_1(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return encoding::snorm_to_float(value.x);
}

float2 Texture_byte_2_snorm::at_2(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float2(encoding::snorm_to_float(value.x),
				  encoding::snorm_to_float(value.y));
}

float3 Texture_byte_2_snorm::at_3(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float3(encoding::snorm_to_float(value.x),
				  encoding::snorm_to_float(value.y),
				  0.f);
}

float4 Texture_byte_2_snorm::at_4(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float4(encoding::snorm_to_float(value.x),
				  encoding::snorm_to_float(value.y),
				  0.f,
				  1.f);
}

float Texture_byte_2_snorm::at_element_1(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return encoding::snorm_to_float(value.x);
}

float2 Texture_byte_2_snorm::at_element_2(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float2(encoding::snorm_to_float(value.x),
				  encoding::snorm_to_float(value.y));
}

float3 Texture_byte_2_snorm::at_element_3(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float3(encoding::snorm_to_float(value.x),
				  encoding::snorm_to_float(value.y),
				  0.f);
}

float4 Texture_byte_2_snorm::at_element_4(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float4(encoding::snorm_to_float(value.x),
				  encoding::snorm_to_float(value.y),
				  0.f,
				  1.f);
}

float Texture_byte_2_snorm::at_1(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return encoding::snorm_to_float(value.x);
}

float2 Texture_byte_2_snorm::at_2(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float2(encoding::snorm_to_float(value.x),
				  encoding::snorm_to_float(value.y));
}

float3 Texture_byte_2_snorm::at_3(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float3(encoding::snorm_to_float(value.x),
				  encoding::snorm_to_float(value.y),
				  0.f);
}

float4 Texture_byte_2_snorm::at_4(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float4(encoding::snorm_to_float(value.x),
				  encoding::snorm_to_float(value.y),
				  0.f,
				  1.f);
}

}}
