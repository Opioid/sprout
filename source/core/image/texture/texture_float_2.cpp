#include "texture_float_2.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Float_2::Float_2(std::shared_ptr<Image> image) :
	Texture(image),
	image_(*static_cast<const image::Float_2*>(image.get())) {}

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

float4 Float_2::gather_1(int4 xy_xy1) const {
	auto v00 = image_.load(xy_xy1.x, xy_xy1.y);
	auto v01 = image_.load(xy_xy1.x, xy_xy1.w);
	auto v10 = image_.load(xy_xy1.z, xy_xy1.y);
	auto v11 = image_.load(xy_xy1.z, xy_xy1.w);

	return float4(v00.x, v01.x, v10.x, v11.x);
}

void Float_2::gather_2(int4 xy_xy1, float2 c[4]) const {
	auto v00 = image_.load(xy_xy1.x, xy_xy1.y);
	auto v01 = image_.load(xy_xy1.x, xy_xy1.w);
	auto v10 = image_.load(xy_xy1.z, xy_xy1.y);
	auto v11 = image_.load(xy_xy1.z, xy_xy1.w);

	c[0] = v00;
	c[1] = v01;
	c[2] = v10;
	c[3] = v11;
}

void Float_2::gather_3(int4 xy_xy1, float3 c[4]) const {
	auto v00 = image_.load(xy_xy1.x, xy_xy1.y);
	auto v01 = image_.load(xy_xy1.x, xy_xy1.w);
	auto v10 = image_.load(xy_xy1.z, xy_xy1.y);
	auto v11 = image_.load(xy_xy1.z, xy_xy1.w);

	c[0] = float3(v00, 0.f);
	c[1] = float3(v01, 0.f);
	c[2] = float3(v10, 0.f);
	c[3] = float3(v11, 0.f);
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
