#include "texture_float_2.hpp"
#include "image/typed_image.inl"
#include "base/math/vector4.inl"

namespace image::texture {

Float2::Float2(std::shared_ptr<Image> image) :
	Texture(image),
	image_(*static_cast<const image::Float2*>(image.get())) {}

float3 Float2::at_3(int32_t i) const {
	return float3(image_.load(i), 0.f);
}

float Float2::at_1(int32_t x, int32_t y) const {
	return image_.load(x, y)[0];
}

float2 Float2::at_2(int32_t x, int32_t y) const {
	return image_.load(x, y);
}

float3 Float2::at_3(int32_t x, int32_t y) const {
	return float3(image_.load(x, y), 0.f);
}

float4 Float2::at_4(int32_t x, int32_t y) const {
	return float4(image_.load(x, y), 0.f, 1.f);
}

float4 Float2::gather_1(const int4& xy_xy1) const {
	float2 v[4];
	image_.gather(xy_xy1, v);

	return float4(v[0][0], v[1][0], v[2][0], v[3][0]);
}

void Float2::gather_2(const int4& xy_xy1, float2 c[4]) const {
	image_.gather(xy_xy1, c);
}

void Float2::gather_3(const int4& xy_xy1, float3 c[4]) const {
	float2 v[4];
	image_.gather(xy_xy1, v);

	c[0] = float3(v[0], 0.f);
	c[1] = float3(v[1], 0.f);
	c[2] = float3(v[2], 0.f);
	c[3] = float3(v[3], 0.f);
}

float Float2::at_element_1(int32_t x, int32_t y, int32_t element) const {
	return image_.at_element(x, y, element)[0];
}

float2 Float2::at_element_2(int32_t x, int32_t y, int32_t element) const {
	return image_.at_element(x, y, element);
}

float3 Float2::at_element_3(int32_t x, int32_t y, int32_t element) const {
	return float3(image_.at_element(x, y, element), 0.f);
}

float4 Float2::at_element_4(int32_t x, int32_t y, int32_t element) const {
	return float4(image_.at_element(x, y, element), 0.f, 1.f);
}

float Float2::at_1(int32_t x, int32_t y, int32_t z) const {
	return image_.load(x, y, z)[0];
}

float2 Float2::at_2(int32_t x, int32_t y, int32_t z) const {
	return image_.load(x, y, z);
}

float3 Float2::at_3(int32_t x, int32_t y, int32_t z) const {
	return float3(image_.load(x, y, z), 0.f);
}

float4 Float2::at_4(int32_t x, int32_t y, int32_t z) const {
	return float4(image_.load(x, y, z), 0.f, 1.f);
}

}
