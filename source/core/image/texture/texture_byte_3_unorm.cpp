#include "texture_byte_3_unorm.hpp"
#include "image/typed_image.inl"
#include "base/spectrum/rgb.inl"
#include "base/math/vector.inl"

namespace image { namespace texture {

Byte_3_unorm::Byte_3_unorm(std::shared_ptr<Image> image) :
	Texture(image),
	image_(*dynamic_cast<const Byte_3*>(image.get())) {}

float Byte_3_unorm::at_1(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return spectrum::unorm_to_float(value.x);
}

float2 Byte_3_unorm::at_2(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float2(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y));
}

float3 Byte_3_unorm::at_3(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float3(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y),
				  spectrum::unorm_to_float(value.z));
}

float4 Byte_3_unorm::at_4(int32_t x, int32_t y) const {
	auto value = image_.load(x, y);
	return float4(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y),
				  spectrum::unorm_to_float(value.z),
				  1.f);
}

float4 Byte_3_unorm::gather_1(int4 xy_xy1) const {
	auto v00 = image_.load(xy_xy1.x, xy_xy1.y);
	auto v01 = image_.load(xy_xy1.x, xy_xy1.w);
	auto v10 = image_.load(xy_xy1.z, xy_xy1.y);
	auto v11 = image_.load(xy_xy1.z, xy_xy1.w);

	return float4(spectrum::unorm_to_float(v00.x),
				  spectrum::unorm_to_float(v01.x),
				  spectrum::unorm_to_float(v10.x),
				  spectrum::unorm_to_float(v11.x));
}

void Byte_3_unorm::gather_2(int4 xy_xy1, float2 c[4]) const {
	auto v00 = image_.load(xy_xy1.x, xy_xy1.y);
	auto v01 = image_.load(xy_xy1.x, xy_xy1.w);
	auto v10 = image_.load(xy_xy1.z, xy_xy1.y);
	auto v11 = image_.load(xy_xy1.z, xy_xy1.w);

	c[0] = float2(spectrum::unorm_to_float(v00.x),
				  spectrum::unorm_to_float(v00.y));

	c[1] = float2(spectrum::unorm_to_float(v01.x),
				  spectrum::unorm_to_float(v01.y));

	c[2] = float2(spectrum::unorm_to_float(v10.x),
				  spectrum::unorm_to_float(v10.y));

	c[3] = float2(spectrum::unorm_to_float(v11.x),
				  spectrum::unorm_to_float(v11.y));
}

void Byte_3_unorm::gather_3(int4 xy_xy1, float3 c[4]) const {
	auto v00 = image_.load(xy_xy1.x, xy_xy1.y);
	auto v01 = image_.load(xy_xy1.x, xy_xy1.w);
	auto v10 = image_.load(xy_xy1.z, xy_xy1.y);
	auto v11 = image_.load(xy_xy1.z, xy_xy1.w);

	c[0] = float3(spectrum::unorm_to_float(v00.x),
				  spectrum::unorm_to_float(v00.y),
				  spectrum::unorm_to_float(v00.z));

	c[1] = float3(spectrum::unorm_to_float(v01.x),
				  spectrum::unorm_to_float(v01.y),
				  spectrum::unorm_to_float(v01.z));

	c[2] = float3(spectrum::unorm_to_float(v10.x),
				  spectrum::unorm_to_float(v10.y),
				  spectrum::unorm_to_float(v10.z));

	c[3] = float3(spectrum::unorm_to_float(v11.x),
				  spectrum::unorm_to_float(v11.y),
				  spectrum::unorm_to_float(v11.z));
}

float Byte_3_unorm::at_element_1(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return spectrum::unorm_to_float(value.x);
}

float2 Byte_3_unorm::at_element_2(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float2(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y));
}

float3 Byte_3_unorm::at_element_3(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float3(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y),
				  spectrum::unorm_to_float(value.z));
}

float4 Byte_3_unorm::at_element_4(int32_t x, int32_t y, int32_t element) const {
	auto value = image_.load_element(x, y, element);
	return float4(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y),
				  spectrum::unorm_to_float(value.z),
				  1.f);
}

float Byte_3_unorm::at_1(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return spectrum::unorm_to_float(value.x);
}

float2 Byte_3_unorm::at_2(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float2(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y));
}

float3 Byte_3_unorm::at_3(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float3(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y),
				  spectrum::unorm_to_float(value.z));
}

float4 Byte_3_unorm::at_4(int32_t x, int32_t y, int32_t z) const {
	auto value = image_.load(x, y, z);
	return float4(spectrum::unorm_to_float(value.x),
				  spectrum::unorm_to_float(value.y),
				  spectrum::unorm_to_float(value.z),
				  1.f);
}

}}
