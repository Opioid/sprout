#include "texture_float_2.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Float_2::Float_2(std::shared_ptr<Image> image) :
	Texture(image),
	image_(*reinterpret_cast<const image::Float_2*>(image.get())) {}

float Float_2::at_1(int32_t x, int32_t y) const {
	return image_.load(x, y).x;
}

float2 Float_2::at_2(int32_t x, int32_t y) const {
	return image_.load(x, y);
}

float3 Float_2::at_3(int32_t x, int32_t y) const {
	return float3(image_.load(x, y), 0.f);
}

float4 Float_2::at_4(int32_t x, int32_t y) const {
	return float4(image_.load(x, y), 0.f, 1.f);
}

float Float_2::at_element_1(int32_t x, int32_t y, int32_t element) const {
	return image_.at_element(x, y, element).x;
}

float2 Float_2::at_element_2(int32_t x, int32_t y, int32_t element) const {
	return image_.at_element(x, y, element);
}

float3 Float_2::at_element_3(int32_t x, int32_t y, int32_t element) const {
	return float3(image_.at_element(x, y, element), 0.f);
}

float4 Float_2::at_element_4(int32_t x, int32_t y, int32_t element) const {
	return float4(image_.at_element(x, y, element), 0.f, 1.f);
}

float Float_2::at_1(int32_t x, int32_t y, int32_t z) const {
	return image_.load(x, y, z).x;
}

float2 Float_2::at_2(int32_t x, int32_t y, int32_t z) const {
	return image_.load(x, y, z);
}

float3 Float_2::at_3(int32_t x, int32_t y, int32_t z) const {
	return float3(image_.load(x, y, z), 0.f);
}

float4 Float_2::at_4(int32_t x, int32_t y, int32_t z) const {
	return float4(image_.load(x, y, z), 0.f, 1.f);
}

}}
