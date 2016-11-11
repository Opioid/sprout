#include "texture_float_3.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_float_3::Texture_float_3(std::shared_ptr<Image> image) :
	Texture(image),
	image_(*reinterpret_cast<const Float_3*>(image.get())) {}

float Texture_float_3::at_1(int32_t x, int32_t y) const {
	return image_.load(x, y).x;
}

float2 Texture_float_3::at_2(int32_t x, int32_t y) const {
	return image_.load(x, y).xy;
}

float3 Texture_float_3::at_3(int32_t x, int32_t y) const {
	return float3(image_.load(x, y));

//	return float3(image_.at(x, y));
}

float4 Texture_float_3::at_4(int32_t x, int32_t y) const {
	return float4(image_.load(x, y), 1.f);
}

float Texture_float_3::at_element_1(int32_t x, int32_t y, int32_t element) const {
	return image_.at_element(x, y, element).x;
}

float2 Texture_float_3::at_element_2(int32_t x, int32_t y, int32_t element) const {
	return image_.at_element(x, y, element).xy;
}

float3 Texture_float_3::at_element_3(int32_t x, int32_t y, int32_t element) const {
//	return image_.at(x, y, element);

	return float3(image_.at_element(x, y, element));
}

float4 Texture_float_3::at_element_4(int32_t x, int32_t y, int32_t element) const {
	return float4(image_.at_element(x, y, element), 1.f);
}

float Texture_float_3::at_1(int32_t x, int32_t y, int32_t z) const {
	return image_.load(x, y, z).x;
}

float2 Texture_float_3::at_2(int32_t x, int32_t y, int32_t z) const {
	return image_.load(x, y, z).xy;
}

float3 Texture_float_3::at_3(int32_t x, int32_t y, int32_t z) const {
	return float3(image_.load(x, y, z));

//	return float3(image_.at(x, y, z));
}

float4 Texture_float_3::at_4(int32_t x, int32_t y, int32_t z) const {
	return float4(image_.load(x, y, z), 1.f);
}

}}
