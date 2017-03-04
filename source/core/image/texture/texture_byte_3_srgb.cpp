#include "texture_byte_3_srgb.hpp"
#include "texture_encoding.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Byte_3_sRGB::Byte_3_sRGB(std::shared_ptr<Image> image) :
	Texture(image),
	image_(*static_cast<const Byte_3*>(image.get())) {}

float Byte_3_sRGB::at_1(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return encoding::cached_srgb_to_float(value.x);
}

float2 Byte_3_sRGB::at_2(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float2(encoding::cached_srgb_to_float(value.x),
				  encoding::cached_srgb_to_float(value.y));
}

float3 Byte_3_sRGB::at_3(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float3(encoding::cached_srgb_to_float(value.x),
				  encoding::cached_srgb_to_float(value.y),
				  encoding::cached_srgb_to_float(value.z));
}

float4 Byte_3_sRGB::at_4(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float4(encoding::cached_srgb_to_float(value.x),
				  encoding::cached_srgb_to_float(value.y),
				  encoding::cached_srgb_to_float(value.z),
				  1.f);
}

float4 Byte_3_sRGB::gather_1(int4 xy_xy1) const {
	auto v00 = image_.load(xy_xy1.v[0], xy_xy1.v[1]);
	auto v01 = image_.load(xy_xy1.v[0], xy_xy1.v[3]);
	auto v10 = image_.load(xy_xy1.v[2], xy_xy1.v[1]);
	auto v11 = image_.load(xy_xy1.v[2], xy_xy1.v[3]);

	return float4(encoding::cached_srgb_to_float(v00.x),
				  encoding::cached_srgb_to_float(v01.x),
				  encoding::cached_srgb_to_float(v10.x),
				  encoding::cached_srgb_to_float(v11.x));
}

void Byte_3_sRGB::gather_2(int4 xy_xy1, float2 c[4]) const {
	auto v00 = image_.load(xy_xy1.v[0], xy_xy1.v[1]);
	auto v01 = image_.load(xy_xy1.v[0], xy_xy1.v[3]);
	auto v10 = image_.load(xy_xy1.v[2], xy_xy1.v[1]);
	auto v11 = image_.load(xy_xy1.v[2], xy_xy1.v[3]);

	c[0] = float2(encoding::cached_srgb_to_float(v00.x),
				  encoding::cached_srgb_to_float(v00.y));

	c[1] = float2(encoding::cached_srgb_to_float(v01.x),
				  encoding::cached_srgb_to_float(v01.y));

	c[2] = float2(encoding::cached_srgb_to_float(v10.x),
				  encoding::cached_srgb_to_float(v10.y));

	c[3] = float2(encoding::cached_srgb_to_float(v11.x),
				  encoding::cached_srgb_to_float(v11.y));
}

void Byte_3_sRGB::gather_3(int4 xy_xy1, float3 c[4]) const {
	auto v00 = image_.load(xy_xy1.v[0], xy_xy1.v[1]);
	auto v01 = image_.load(xy_xy1.v[0], xy_xy1.v[3]);
	auto v10 = image_.load(xy_xy1.v[2], xy_xy1.v[1]);
	auto v11 = image_.load(xy_xy1.v[2], xy_xy1.v[3]);

	c[0] = float3(encoding::cached_srgb_to_float(v00.x),
				  encoding::cached_srgb_to_float(v00.y),
				  encoding::cached_srgb_to_float(v00.z));

	c[1] = float3(encoding::cached_srgb_to_float(v01.x),
				  encoding::cached_srgb_to_float(v01.y),
				  encoding::cached_srgb_to_float(v01.z));

	c[2] = float3(encoding::cached_srgb_to_float(v10.x),
				  encoding::cached_srgb_to_float(v10.y),
				  encoding::cached_srgb_to_float(v10.z));

	c[3] = float3(encoding::cached_srgb_to_float(v11.x),
				  encoding::cached_srgb_to_float(v11.y),
				  encoding::cached_srgb_to_float(v11.z));
}

float Byte_3_sRGB::at_element_1(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return encoding::cached_srgb_to_float(value.x);
}

float2 Byte_3_sRGB::at_element_2(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float2(encoding::cached_srgb_to_float(value.x),
				  encoding::cached_srgb_to_float(value.y));
}

float3 Byte_3_sRGB::at_element_3(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float3(encoding::cached_srgb_to_float(value.x),
				  encoding::cached_srgb_to_float(value.y),
				  encoding::cached_srgb_to_float(value.z));
}

float4 Byte_3_sRGB::at_element_4(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float4(encoding::cached_srgb_to_float(value.x),
				  encoding::cached_srgb_to_float(value.y),
				  encoding::cached_srgb_to_float(value.z),
				  1.f);
}

float Byte_3_sRGB::at_1(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return encoding::cached_srgb_to_float(value.x);
}

float2 Byte_3_sRGB::at_2(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float2(encoding::cached_srgb_to_float(value.x),
				  encoding::cached_srgb_to_float(value.y));
}

float3 Byte_3_sRGB::at_3(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float3(encoding::cached_srgb_to_float(value.x),
				  encoding::cached_srgb_to_float(value.y),
				  encoding::cached_srgb_to_float(value.z));
}

float4 Byte_3_sRGB::at_4(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float4(encoding::cached_srgb_to_float(value.x),
				  encoding::cached_srgb_to_float(value.y),
				  encoding::cached_srgb_to_float(value.z),
				  1.f);
}

}}
