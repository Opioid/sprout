#include "texture_float_3.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Float_3::Float_3(std::shared_ptr<Image> image) :
	Texture(image),
	image_(*reinterpret_cast<const image::Float_3*>(image.get())) {}

float Float_3::at_1(int32_t x, int32_t y) const {
	return image_.load(x, y).x;
}

float2 Float_3::at_2(int32_t x, int32_t y) const {
	return image_.load(x, y).xy;
}

float3 Float_3::at_3(int32_t x, int32_t y) const {
	return float3(image_.load(x, y));

//	return float3(image_.at(x, y));
}

float4 Float_3::at_4(int32_t x, int32_t y) const {
	return float4(image_.load(x, y), 1.f);
}

void Float_3::gather_3(int4 xy_xy1, float3 c[4]) const {
	auto v00 = image_.load(xy_xy1.x, xy_xy1.y);
	auto v01 = image_.load(xy_xy1.x, xy_xy1.w);
	auto v10 = image_.load(xy_xy1.z, xy_xy1.y);
	auto v11 = image_.load(xy_xy1.z, xy_xy1.w);

	c[0] = float3(v00);
	c[1] = float3(v01);
	c[2] = float3(v10);
	c[3] = float3(v11);
}

float Float_3::at_element_1(int32_t x, int32_t y, int32_t element) const {
	return image_.at_element(x, y, element).x;
}

float2 Float_3::at_element_2(int32_t x, int32_t y, int32_t element) const {
	return image_.at_element(x, y, element).xy;
}

float3 Float_3::at_element_3(int32_t x, int32_t y, int32_t element) const {
//	return image_.at(x, y, element);

	return float3(image_.at_element(x, y, element));
}

float4 Float_3::at_element_4(int32_t x, int32_t y, int32_t element) const {
	return float4(image_.at_element(x, y, element), 1.f);
}

float Float_3::at_1(int32_t x, int32_t y, int32_t z) const {
	return image_.load(x, y, z).x;
}

float2 Float_3::at_2(int32_t x, int32_t y, int32_t z) const {
	return image_.load(x, y, z).xy;
}

float3 Float_3::at_3(int32_t x, int32_t y, int32_t z) const {
	return float3(image_.load(x, y, z));

//	return float3(image_.at(x, y, z));
}

float4 Float_3::at_4(int32_t x, int32_t y, int32_t z) const {
	return float4(image_.load(x, y, z), 1.f);
}

}}
