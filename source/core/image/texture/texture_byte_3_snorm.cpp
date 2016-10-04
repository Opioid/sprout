#include "texture_byte_3_snorm.hpp"
#include "texture_encoding.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_byte_3_snorm::Texture_byte_3_snorm(std::shared_ptr<Image> image) :
	Texture(image),
	image_(*dynamic_cast<const Image_byte_3*>(image.get())) {}

float Texture_byte_3_snorm::at_1(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return encoding::snorm_to_float(value.x);
}

float2 Texture_byte_3_snorm::at_2(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float2(encoding::snorm_to_float(value.x),
				  encoding::snorm_to_float(value.y));
}

float3 Texture_byte_3_snorm::at_3(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float3(encoding::snorm_to_float(value.x),
				  encoding::snorm_to_float(value.y),
				  encoding::snorm_to_float(value.z));
}

float4 Texture_byte_3_snorm::at_4(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float4(encoding::snorm_to_float(value.x),
				  encoding::snorm_to_float(value.y),
				  encoding::snorm_to_float(value.z),
				  1.f);
}

float Texture_byte_3_snorm::at_1(int32_t x, int32_t y, int32_t element) const {
	auto& value = image_.at(x, y, element);
	return encoding::snorm_to_float(value.x);
}

float2 Texture_byte_3_snorm::at_2(int32_t x, int32_t y, int32_t element) const {
	auto& value = image_.at(x, y, element);
	return float2(encoding::snorm_to_float(value.x),
				  encoding::snorm_to_float(value.y));
}

float3 Texture_byte_3_snorm::at_3(int32_t x, int32_t y, int32_t element) const {
	auto& value = image_.at(x, y, element);
	return float3(encoding::snorm_to_float(value.x),
				  encoding::snorm_to_float(value.y),
				  encoding::snorm_to_float(value.z));
}

float4 Texture_byte_3_snorm::at_4(int32_t x, int32_t y, int32_t element) const {
	auto& value = image_.at(x, y, element);
	return float4(encoding::snorm_to_float(value.x),
				  encoding::snorm_to_float(value.y),
				  encoding::snorm_to_float(value.z),
				  1.f);
}

}}
