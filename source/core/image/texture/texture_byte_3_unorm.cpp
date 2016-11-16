#include "texture_byte_3_unorm.hpp"
#include "image/typed_image.inl"
#include "base/spectrum/rgb.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Byte_3_unorm::Byte_3_unorm(std::shared_ptr<Image> image) :
	Texture(image),
	image_(*dynamic_cast<const Byte_3*>(image.get())) {}

float Byte_3_unorm::at_1(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return spectrum::unorm_to_float(value.x);
}

float2 Byte_3_unorm::at_2(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float2(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y));
}

float3 Byte_3_unorm::at_3(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float3(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y),
				  spectrum::unorm_to_float(value.z));
}

float4 Byte_3_unorm::at_4(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float4(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y),
				  spectrum::unorm_to_float(value.z),
				  1.f);
}

float Byte_3_unorm::at_element_1(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return spectrum::unorm_to_float(value.x);
}

float2 Byte_3_unorm::at_element_2(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float2(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y));
}

float3 Byte_3_unorm::at_element_3(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float3(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y),
				  spectrum::unorm_to_float(value.z));
}

float4 Byte_3_unorm::at_element_4(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float4(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y),
				  spectrum::unorm_to_float(value.z),
				  1.f);
}

float Byte_3_unorm::at_1(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return spectrum::unorm_to_float(value.x);
}

float2 Byte_3_unorm::at_2(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float2(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y));
}

float3 Byte_3_unorm::at_3(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float3(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y),
				  spectrum::unorm_to_float(value.z));
}

float4 Byte_3_unorm::at_4(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float4(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y),
				  spectrum::unorm_to_float(value.z),
				  1.f);
}

}}
