#include "texture_byte_3_snorm.hpp"
#include "texture_encoding.hpp"
#include "image/typed_image.inl"
#include "base/math/vector4.inl"

namespace image { namespace texture {

Byte_3_snorm::Byte_3_snorm(std::shared_ptr<Image> image) :
	Texture(image),
	image_(*static_cast<const Byte_3*>(image.get())) {}

float Byte_3_snorm::at_1(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return encoding::cached_snorm_to_float(value[0]);
}

float2 Byte_3_snorm::at_2(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float2(encoding::cached_snorm_to_float(value[0]),
				  encoding::cached_snorm_to_float(value[1]));
}

float3 Byte_3_snorm::at_3(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float3(encoding::cached_snorm_to_float(value[0]),
				  encoding::cached_snorm_to_float(value[1]),
				  encoding::cached_snorm_to_float(value[2]));
}

float4 Byte_3_snorm::at_4(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float4(encoding::cached_snorm_to_float(value[0]),
				  encoding::cached_snorm_to_float(value[1]),
				  encoding::cached_snorm_to_float(value[2]),
				  1.f);
}

float4 Byte_3_snorm::gather_1(int4 xy_xy1) const {
	byte3 v[4];
	image_.gather(xy_xy1, v);

	return float4(encoding::cached_snorm_to_float(v[0][0]),
				  encoding::cached_snorm_to_float(v[1][0]),
				  encoding::cached_snorm_to_float(v[2][0]),
				  encoding::cached_snorm_to_float(v[3][0]));
}

void Byte_3_snorm::gather_2(int4 xy_xy1, float2 c[4]) const {
	byte3 v[4];
	image_.gather(xy_xy1, v);

	c[0] = float2(encoding::cached_snorm_to_float(v[0][0]),
				  encoding::cached_snorm_to_float(v[0][1]));

	c[1] = float2(encoding::cached_snorm_to_float(v[1][0]),
				  encoding::cached_snorm_to_float(v[1][1]));

	c[2] = float2(encoding::cached_snorm_to_float(v[2][0]),
				  encoding::cached_snorm_to_float(v[2][1]));

	c[3] = float2(encoding::cached_snorm_to_float(v[3][0]),
				  encoding::cached_snorm_to_float(v[3][1]));
}

void Byte_3_snorm::gather_3(int4 xy_xy1, float3 c[4]) const {
	byte3 v[4];
	image_.gather(xy_xy1, v);

	c[0] = float3(encoding::cached_snorm_to_float(v[0][0]),
				  encoding::cached_snorm_to_float(v[0][1]),
				  encoding::cached_snorm_to_float(v[0][2]));

	c[1] = float3(encoding::cached_snorm_to_float(v[1][0]),
				  encoding::cached_snorm_to_float(v[1][1]),
				  encoding::cached_snorm_to_float(v[1][2]));

	c[2] = float3(encoding::cached_snorm_to_float(v[2][0]),
				  encoding::cached_snorm_to_float(v[2][1]),
				  encoding::cached_snorm_to_float(v[2][2]));

	c[3] = float3(encoding::cached_snorm_to_float(v[3][0]),
				  encoding::cached_snorm_to_float(v[3][1]),
				  encoding::cached_snorm_to_float(v[3][2]));
}

float Byte_3_snorm::at_element_1(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return encoding::cached_snorm_to_float(value[0]);
}

float2 Byte_3_snorm::at_element_2(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float2(encoding::cached_snorm_to_float(value[0]),
				  encoding::cached_snorm_to_float(value[1]));
}

float3 Byte_3_snorm::at_element_3(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float3(encoding::cached_snorm_to_float(value[0]),
				  encoding::cached_snorm_to_float(value[1]),
				  encoding::cached_snorm_to_float(value[2]));
}

float4 Byte_3_snorm::at_element_4(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float4(encoding::cached_snorm_to_float(value[0]),
				  encoding::cached_snorm_to_float(value[1]),
				  encoding::cached_snorm_to_float(value[2]),
				  1.f);
}

float Byte_3_snorm::at_1(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return encoding::cached_snorm_to_float(value[0]);
}

float2 Byte_3_snorm::at_2(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float2(encoding::cached_snorm_to_float(value[0]),
				  encoding::cached_snorm_to_float(value[1]));
}

float3 Byte_3_snorm::at_3(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float3(encoding::cached_snorm_to_float(value[0]),
				  encoding::cached_snorm_to_float(value[1]),
				  encoding::cached_snorm_to_float(value[2]));
}

float4 Byte_3_snorm::at_4(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float4(encoding::cached_snorm_to_float(value[0]),
				  encoding::cached_snorm_to_float(value[1]),
				  encoding::cached_snorm_to_float(value[2]),
				  1.f);
}

}}
