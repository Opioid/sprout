#include "texture_byte_1_unorm.hpp"
#include "texture_encoding.hpp"
#include "image/typed_image.inl"
#include "base/math/vector4.inl"

namespace image::texture {

Byte1_unorm::Byte1_unorm(std::shared_ptr<Image> const& image) :
	Texture(image),
	image_(*static_cast<const Byte1*>(image.get())) {}

float Byte1_unorm::at_1(int32_t i) const {
	uint8_t value = image_.load(i);
	return encoding::cached_unorm_to_float(value);
}

float3 Byte1_unorm::at_3(int32_t i) const {
	uint8_t value = image_.load(i);
	return float3(encoding::cached_unorm_to_float(value), 0.f, 0.f);
}

float Byte1_unorm::at_1(int32_t x, int32_t y) const {
	uint8_t value = image_.load(x, y);
	return encoding::cached_unorm_to_float(value);
}

float2 Byte1_unorm::at_2(int32_t x, int32_t y) const {
	uint8_t value = image_.load(x, y);
	return float2(encoding::cached_unorm_to_float(value), 0.f);
}

float3 Byte1_unorm::at_3(int32_t x, int32_t y) const {
	uint8_t value = image_.load(x, y);
	return float3(encoding::cached_unorm_to_float(value), 0.f, 0.f);
}

float4 Byte1_unorm::at_4(int32_t x, int32_t y) const {
	uint8_t value = image_.load(x, y);
	return float4(encoding::cached_unorm_to_float(value), 0.f, 0.f, 1.f);
}

float4 Byte1_unorm::gather_1(int4 const& xy_xy1) const {
	uint8_t v[4];
	image_.gather(xy_xy1, v);

	return float4(encoding::cached_unorm_to_float(v[0]),
				  encoding::cached_unorm_to_float(v[1]),
				  encoding::cached_unorm_to_float(v[2]),
				  encoding::cached_unorm_to_float(v[3]));
}

void Byte1_unorm::gather_2(int4 const& xy_xy1, float2 c[4]) const {
	uint8_t v[4];
	image_.gather(xy_xy1, v);

	c[0] = float2(encoding::cached_unorm_to_float(v[0]), 0.f);
	c[1] = float2(encoding::cached_unorm_to_float(v[1]), 0.f);
	c[2] = float2(encoding::cached_unorm_to_float(v[2]), 0.f);
	c[3] = float2(encoding::cached_unorm_to_float(v[3]), 0.f);
}

void Byte1_unorm::gather_3(int4 const& xy_xy1, float3 c[4]) const {
	uint8_t v[4];
	image_.gather(xy_xy1, v);

	c[0] = float3(encoding::cached_unorm_to_float(v[0]), 0.f, 0.f);
	c[1] = float3(encoding::cached_unorm_to_float(v[1]), 0.f, 0.f);
	c[2] = float3(encoding::cached_unorm_to_float(v[2]), 0.f, 0.f);
	c[3] = float3(encoding::cached_unorm_to_float(v[3]), 0.f, 0.f);
}

float Byte1_unorm::at_element_1(int32_t x, int32_t y, int32_t element) const {
	uint8_t value = image_.load_element(x, y, element);
	return encoding::cached_unorm_to_float(value);
}

float2 Byte1_unorm::at_element_2(int32_t x, int32_t y, int32_t element) const {
	uint8_t value = image_.load_element(x, y, element);
	return float2(encoding::cached_unorm_to_float(value), 0.f);
}

float3 Byte1_unorm::at_element_3(int32_t x, int32_t y, int32_t element) const {
	uint8_t value = image_.load_element(x, y, element);
	return float3(encoding::cached_unorm_to_float(value), 0.f, 0.f);
}

float4 Byte1_unorm::at_element_4(int32_t x, int32_t y, int32_t element) const {
	uint8_t value = image_.load_element(x, y, element);
	return float4(encoding::cached_unorm_to_float(value), 0.f, 0.f, 1.f);
}

float Byte1_unorm::at_1(int32_t x, int32_t y, int32_t z) const {
	uint8_t value = image_.load(x, y, z);
	return encoding::cached_unorm_to_float(value);
}

float2 Byte1_unorm::at_2(int32_t x, int32_t y, int32_t z) const {
	uint8_t value = image_.load(x, y, z);
	return float2(encoding::cached_unorm_to_float(value), 0.f);
}

float3 Byte1_unorm::at_3(int32_t x, int32_t y, int32_t z) const {
	uint8_t value = image_.load(x, y, z);
	return float3(encoding::cached_unorm_to_float(value), 0.f, 0.f);
}

float4 Byte1_unorm::at_4(int32_t x, int32_t y, int32_t z) const {
	uint8_t value = image_.load(x, y, z);
	return float4(encoding::cached_unorm_to_float(value), 0.f, 0.f, 1.f);
}

}
