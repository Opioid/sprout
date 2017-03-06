#include "texture_float_1.hpp"
#include "image/typed_image.inl"
#include "base/math/vector4.inl"

namespace image { namespace texture {

Float_1::Float_1(std::shared_ptr<Image> image) :
	Texture(image),
	image_(*reinterpret_cast<const image::Float_1*>(image.get())) {}

float Float_1::at_1(int32_t x, int32_t y) const {
	return image_.load(x, y);
}

float2 Float_1::at_2(int32_t x, int32_t y) const {
	return float2(image_.load(x, y), 0.f);
}

float3 Float_1::at_3(int32_t x, int32_t y) const {
	return float3(image_.load(x, y), 0.f, 0.f);
}

float4 Float_1::at_4(int32_t x, int32_t y) const {
	return float4(image_.load(x, y), 0.f, 0.f, 1.f);
}

float4 Float_1::gather_1(int4 xy_xy1) const {
	auto v00 = image_.load(xy_xy1[0], xy_xy1[1]);
	auto v01 = image_.load(xy_xy1[0], xy_xy1[3]);
	auto v10 = image_.load(xy_xy1[2], xy_xy1[1]);
	auto v11 = image_.load(xy_xy1[2], xy_xy1[3]);

	return float4(v00, v01, v10, v11);
}

void Float_1::gather_2(int4 xy_xy1, float2 c[4]) const {
	auto v00 = image_.load(xy_xy1[0], xy_xy1[1]);
	auto v01 = image_.load(xy_xy1[0], xy_xy1[3]);
	auto v10 = image_.load(xy_xy1[2], xy_xy1[1]);
	auto v11 = image_.load(xy_xy1[2], xy_xy1[3]);

	c[0] = float2(v00, 0.f);
	c[1] = float2(v01, 0.f);
	c[2] = float2(v10, 0.f);
	c[3] = float2(v11, 0.f);
}

void Float_1::gather_3(int4 xy_xy1, float3 c[4]) const {
	auto v00 = image_.load(xy_xy1[0], xy_xy1[1]);
	auto v01 = image_.load(xy_xy1[0], xy_xy1[3]);
	auto v10 = image_.load(xy_xy1[2], xy_xy1[1]);
	auto v11 = image_.load(xy_xy1[2], xy_xy1[3]);

	c[0] = float3(v00, 0.f, 0.f);
	c[1] = float3(v01, 0.f, 0.f);
	c[2] = float3(v10, 0.f, 0.f);
	c[3] = float3(v11, 0.f, 0.f);
}

float Float_1::at_element_1(int32_t x, int32_t y, int32_t element) const {
	return image_.at_element(x, y, element);
}

float2 Float_1::at_element_2(int32_t x, int32_t y, int32_t element) const {
	return float2(image_.at_element(x, y, element), 0.f);
}

float3 Float_1::at_element_3(int32_t x, int32_t y, int32_t element) const {
	return float3(image_.at_element(x, y, element), 0.f, 0.f);
}

float4 Float_1::at_element_4(int32_t x, int32_t y, int32_t element) const {
	return float4(image_.at_element(x, y, element), 0.f, 0.f, 1.f);
}

float Float_1::at_1(int32_t x, int32_t y, int32_t z) const {
	return image_.load(x, y, z);
}

float2 Float_1::at_2(int32_t x, int32_t y, int32_t z) const {
	return float2(image_.load(x, y, z), 0.f);
}

float3 Float_1::at_3(int32_t x, int32_t y, int32_t z) const {
	return float3(image_.load(x, y, z), 0.f, 0.f);
}

float4 Float_1::at_4(int32_t x, int32_t y, int32_t z) const {
	return float4(image_.load(x, y, z), 0.f, 0.f, 1.f);
}

}}
