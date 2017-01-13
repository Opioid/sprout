#include "texture_byte_2_unorm.hpp"
#include "texture_encoding.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Byte_2_unorm::Byte_2_unorm(std::shared_ptr<Image> image) :
	Texture(image),
	image_(*dynamic_cast<const Byte_2*>(image.get())) {}

float Byte_2_unorm::at_1(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return encoding::unorm_to_float(value.x);
}

float2 Byte_2_unorm::at_2(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float2(encoding::unorm_to_float(value.x),
				  encoding::unorm_to_float(value.y));
}

float3 Byte_2_unorm::at_3(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float3(encoding::unorm_to_float(value.x),
				  encoding::unorm_to_float(value.y),
				  0.f);
}

float4 Byte_2_unorm::at_4(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float4(encoding::unorm_to_float(value.x),
				  encoding::unorm_to_float(value.y),
				  0.f,
				  1.f);
}

float4 Byte_2_unorm::gather_1(int4 xy_xy1) const {
	auto v00 = image_.load(xy_xy1.x, xy_xy1.y);
	auto v01 = image_.load(xy_xy1.x, xy_xy1.w);
	auto v10 = image_.load(xy_xy1.z, xy_xy1.y);
	auto v11 = image_.load(xy_xy1.z, xy_xy1.w);

	return float4(encoding::unorm_to_float(v00.x),
				  encoding::unorm_to_float(v01.x),
				  encoding::unorm_to_float(v10.x),
				  encoding::unorm_to_float(v11.x));
}

void Byte_2_unorm::gather_2(int4 xy_xy1, float2 c[4]) const {
	auto v00 = image_.load(xy_xy1.x, xy_xy1.y);
	auto v01 = image_.load(xy_xy1.x, xy_xy1.w);
	auto v10 = image_.load(xy_xy1.z, xy_xy1.y);
	auto v11 = image_.load(xy_xy1.z, xy_xy1.w);

	c[0] = float2(encoding::unorm_to_float(v00.x),
				  encoding::unorm_to_float(v00.y));

	c[1] = float2(encoding::unorm_to_float(v01.x),
				  encoding::unorm_to_float(v01.y));

	c[2] = float2(encoding::unorm_to_float(v10.x),
				  encoding::unorm_to_float(v10.y));

	c[3] = float2(encoding::unorm_to_float(v11.x),
				  encoding::unorm_to_float(v11.y));
}

void Byte_2_unorm::gather_3(int4 xy_xy1, float3 c[4]) const {
	auto v00 = image_.load(xy_xy1.x, xy_xy1.y);
	auto v01 = image_.load(xy_xy1.x, xy_xy1.w);
	auto v10 = image_.load(xy_xy1.z, xy_xy1.y);
	auto v11 = image_.load(xy_xy1.z, xy_xy1.w);

	c[0] = float3(encoding::unorm_to_float(v00.x),
				  encoding::unorm_to_float(v00.y),
				  0.f);

	c[1] = float3(encoding::unorm_to_float(v01.x),
				  encoding::unorm_to_float(v01.y),
				  0.f);

	c[2] = float3(encoding::unorm_to_float(v10.x),
				  encoding::unorm_to_float(v10.y),
				  0.f);

	c[3] = float3(encoding::unorm_to_float(v11.x),
				  encoding::unorm_to_float(v11.y),
				  0.f);
}

float Byte_2_unorm::at_element_1(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return encoding::unorm_to_float(value.x);
}

float2 Byte_2_unorm::at_element_2(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float2(encoding::unorm_to_float(value.x),
				  encoding::unorm_to_float(value.y));
}

float3 Byte_2_unorm::at_element_3(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float3(encoding::unorm_to_float(value.x),
				  encoding::unorm_to_float(value.y),
				  0.f);
}

float4 Byte_2_unorm::at_element_4(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float4(encoding::unorm_to_float(value.x),
				  encoding::unorm_to_float(value.y),
				  0.f,
				  1.f);
}

float Byte_2_unorm::at_1(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return encoding::unorm_to_float(value.x);
}

float2 Byte_2_unorm::at_2(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float2(encoding::unorm_to_float(value.x),
				  encoding::unorm_to_float(value.y));
}

float3 Byte_2_unorm::at_3(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float3(encoding::unorm_to_float(value.x),
				  encoding::unorm_to_float(value.y),
				  0.f);
}

float4 Byte_2_unorm::at_4(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float4(encoding::unorm_to_float(value.x),
				  encoding::unorm_to_float(value.y),
				  0.f,
				  1.f);
}

}}
