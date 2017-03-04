#include "texture_byte_1_unorm.hpp"
#include "texture_encoding.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Byte_1_unorm::Byte_1_unorm(std::shared_ptr<Image> image) :
	Texture(image),
	image_(*static_cast<const Byte_1*>(image.get())) {}

float Byte_1_unorm::at_1(int32_t x, int32_t y) const {
	uint8_t value = image_.load(x, y);
	return encoding::cached_unorm_to_float(value);
}

float2 Byte_1_unorm::at_2(int32_t x, int32_t y) const {
	uint8_t value = image_.load(x, y);
	return float2(encoding::cached_unorm_to_float(value), 0.f);
}

float3 Byte_1_unorm::at_3(int32_t x, int32_t y) const {
	uint8_t value = image_.load(x, y);
	return float3(encoding::cached_unorm_to_float(value), 0.f, 0.f);
}

float4 Byte_1_unorm::at_4(int32_t x, int32_t y) const {
	uint8_t value = image_.load(x, y);
	return float4(encoding::cached_unorm_to_float(value), 0.f, 0.f, 1.f);
}

float4 Byte_1_unorm::gather_1(int4 xy_xy1) const {
	const auto v00 = image_.load(xy_xy1.v[0], xy_xy1.v[1]);
	const auto v01 = image_.load(xy_xy1.v[0], xy_xy1.v[3]);
	const auto v10 = image_.load(xy_xy1.v[2], xy_xy1.v[1]);
	const auto v11 = image_.load(xy_xy1.v[2], xy_xy1.v[3]);

	return float4(encoding::cached_unorm_to_float(v00),
				  encoding::cached_unorm_to_float(v01),
				  encoding::cached_unorm_to_float(v10),
				  encoding::cached_unorm_to_float(v11));
}

void Byte_1_unorm::gather_2(int4 xy_xy1, float2 c[4]) const {
	const auto v00 = image_.load(xy_xy1.v[0], xy_xy1.v[1]);
	const auto v01 = image_.load(xy_xy1.v[0], xy_xy1.v[3]);
	const auto v10 = image_.load(xy_xy1.v[2], xy_xy1.v[1]);
	const auto v11 = image_.load(xy_xy1.v[2], xy_xy1.v[3]);

	c[0] = float2(encoding::cached_unorm_to_float(v00), 0.f);
	c[1] = float2(encoding::cached_unorm_to_float(v01), 0.f);
	c[2] = float2(encoding::cached_unorm_to_float(v10), 0.f);
	c[3] = float2(encoding::cached_unorm_to_float(v11), 0.f);
}

void Byte_1_unorm::gather_3(int4 xy_xy1, float3 c[4]) const {
	auto v00 = image_.load(xy_xy1.v[0], xy_xy1.v[1]);
	auto v01 = image_.load(xy_xy1.v[0], xy_xy1.v[3]);
	auto v10 = image_.load(xy_xy1.v[2], xy_xy1.v[1]);
	auto v11 = image_.load(xy_xy1.v[2], xy_xy1.v[3]);

	c[0] = float3(encoding::cached_unorm_to_float(v00), 0.f, 0.f);
	c[1] = float3(encoding::cached_unorm_to_float(v01), 0.f, 0.f);
	c[2] = float3(encoding::cached_unorm_to_float(v10), 0.f, 0.f);
	c[3] = float3(encoding::cached_unorm_to_float(v11), 0.f, 0.f);
}

float Byte_1_unorm::at_element_1(int32_t x, int32_t y, int32_t element) const {
	uint8_t value = image_.load_element(x, y, element);
	return encoding::cached_unorm_to_float(value);
}

float2 Byte_1_unorm::at_element_2(int32_t x, int32_t y, int32_t element) const {
	uint8_t value = image_.load_element(x, y, element);
	return float2(encoding::cached_unorm_to_float(value), 0.f);
}

float3 Byte_1_unorm::at_element_3(int32_t x, int32_t y, int32_t element) const {
	uint8_t value = image_.load_element(x, y, element);
	return float3(encoding::cached_unorm_to_float(value), 0.f, 0.f);
}

float4 Byte_1_unorm::at_element_4(int32_t x, int32_t y, int32_t element) const {
	uint8_t value = image_.load_element(x, y, element);
	return float4(encoding::cached_unorm_to_float(value), 0.f, 0.f, 1.f);
}

float Byte_1_unorm::at_1(int32_t x, int32_t y, int32_t z) const {
	uint8_t value = image_.load(x, y, z);
	return encoding::cached_unorm_to_float(value);
}

float2 Byte_1_unorm::at_2(int32_t x, int32_t y, int32_t z) const {
	uint8_t value = image_.load(x, y, z);
	return float2(encoding::cached_unorm_to_float(value), 0.f);
}

float3 Byte_1_unorm::at_3(int32_t x, int32_t y, int32_t z) const {
	uint8_t value = image_.load(x, y, z);
	return float3(encoding::cached_unorm_to_float(value), 0.f, 0.f);
}

float4 Byte_1_unorm::at_4(int32_t x, int32_t y, int32_t z) const {
	uint8_t value = image_.load(x, y, z);
	return float4(encoding::cached_unorm_to_float(value), 0.f, 0.f, 1.f);
}

}}
