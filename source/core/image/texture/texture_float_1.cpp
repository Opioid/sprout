#include "texture_float_1.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_float_1::Texture_float_1(std::shared_ptr<Image> image) :
	Texture(image),
	image_(*reinterpret_cast<const Float_1*>(image.get())) {}

float Texture_float_1::at_1(int32_t x, int32_t y) const {
	return image_.load(x, y);
}

float2 Texture_float_1::at_2(int32_t x, int32_t y) const {
	return float2(image_.load(x, y), 0.f);
}

float3 Texture_float_1::at_3(int32_t x, int32_t y) const {
	return float3(image_.load(x, y), 0.f, 0.f);
}

float4 Texture_float_1::at_4(int32_t x, int32_t y) const {
	return float4(image_.load(x, y), 0.f, 0.f, 1.f);
}

float Texture_float_1::at_element_1(int32_t x, int32_t y, int32_t element) const {
	return image_.at_element(x, y, element);
}

float2 Texture_float_1::at_element_2(int32_t x, int32_t y, int32_t element) const {
	return float2(image_.at_element(x, y, element), 0.f);
}

float3 Texture_float_1::at_element_3(int32_t x, int32_t y, int32_t element) const {
	return float3(image_.at_element(x, y, element), 0.f, 0.f);
}

float4 Texture_float_1::at_element_4(int32_t x, int32_t y, int32_t element) const {
	return float4(image_.at_element(x, y, element), 0.f, 0.f, 1.f);
}

float Texture_float_1::at_1(int32_t x, int32_t y, int32_t z) const {
	return image_.load(x, y, z);
}

float2 Texture_float_1::at_2(int32_t x, int32_t y, int32_t z) const {
	return float2(image_.load(x, y, z), 0.f);
}

float3 Texture_float_1::at_3(int32_t x, int32_t y, int32_t z) const {
	return float3(image_.load(x, y, z), 0.f, 0.f);
}

float4 Texture_float_1::at_4(int32_t x, int32_t y, int32_t z) const {
	return float4(image_.load(x, y, z), 0.f, 0.f, 1.f);
}

}}
