#include "texture_float_1.hpp"
#include "image/typed_image.inl"
#include "base/math/vector4.inl"

namespace image::texture {

Float1::Float1(const std::shared_ptr<Image>& image) :
	Texture(image),
	image_(*reinterpret_cast<const image::Float1*>(image.get())) {}

float3 Float1::at_3(int32_t i) const {
	return float3(image_.load(i), 0.f, 0.f);
}

float Float1::at_1(int32_t x, int32_t y) const {
	return image_.load(x, y);
}

float2 Float1::at_2(int32_t x, int32_t y) const {
	return float2(image_.load(x, y), 0.f);
}

float3 Float1::at_3(int32_t x, int32_t y) const {
	return float3(image_.load(x, y), 0.f, 0.f);
}

float4 Float1::at_4(int32_t x, int32_t y) const {
	return float4(image_.load(x, y), 0.f, 0.f, 1.f);
}

float4 Float1::gather_1(const int4& xy_xy1) const {
	float v[4];
	image_.gather(xy_xy1, v);

	return float4(v[0], v[1], v[2], v[3]);
}

void Float1::gather_2(const int4& xy_xy1, float2 c[4]) const {
	float v[4];
	image_.gather(xy_xy1, v);

	c[0] = float2(v[0], 0.f);
	c[1] = float2(v[1], 0.f);
	c[2] = float2(v[2], 0.f);
	c[3] = float2(v[3], 0.f);
}

void Float1::gather_3(const int4& xy_xy1, float3 c[4]) const {
	float v[4];
	image_.gather(xy_xy1, v);

	c[0] = float3(v[0], 0.f, 0.f);
	c[1] = float3(v[1], 0.f, 0.f);
	c[2] = float3(v[2], 0.f, 0.f);
	c[3] = float3(v[3], 0.f, 0.f);
}

float Float1::at_element_1(int32_t x, int32_t y, int32_t element) const {
	return image_.at_element(x, y, element);
}

float2 Float1::at_element_2(int32_t x, int32_t y, int32_t element) const {
	return float2(image_.at_element(x, y, element), 0.f);
}

float3 Float1::at_element_3(int32_t x, int32_t y, int32_t element) const {
	return float3(image_.at_element(x, y, element), 0.f, 0.f);
}

float4 Float1::at_element_4(int32_t x, int32_t y, int32_t element) const {
	return float4(image_.at_element(x, y, element), 0.f, 0.f, 1.f);
}

float Float1::at_1(int32_t x, int32_t y, int32_t z) const {
	return image_.load(x, y, z);
}

float2 Float1::at_2(int32_t x, int32_t y, int32_t z) const {
	return float2(image_.load(x, y, z), 0.f);
}

float3 Float1::at_3(int32_t x, int32_t y, int32_t z) const {
	return float3(image_.load(x, y, z), 0.f, 0.f);
}

float4 Float1::at_4(int32_t x, int32_t y, int32_t z) const {
	return float4(image_.load(x, y, z), 0.f, 0.f, 1.f);
}

}
