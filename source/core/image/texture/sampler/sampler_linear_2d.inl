#pragma once

#include "sampler_linear_2d.hpp"
#include "address_mode.hpp"
#include "bilinear.hpp"
#include "image/texture/texture.hpp"
#include <algorithm>

namespace image { namespace texture { namespace sampler {

template<typename Address_U, typename Address_V>
float Linear_2D<Address_U, Address_V>::sample_1(const Texture& texture, float2 uv) const {
	int4 xy_xy1;
	float2 st = map(texture, uv, xy_xy1);

	float4 c = texture.gather_1(xy_xy1);

	return bilinear(c, st[0], st[1]);
}

template<typename Address_U, typename Address_V>
float2 Linear_2D<Address_U, Address_V>::sample_2(const Texture& texture, float2 uv) const {
	int4 xy_xy1;
	float2 st = map(texture, uv, xy_xy1);

	float2 c[4];
	texture.gather_2(xy_xy1, c);

	return bilinear(c, st[0], st[1]);
}

template<typename Address_U, typename Address_V>
template<uint32_t Axis>
float2 Linear_2D<Address_U, Address_V>::sample_2(const Texture& texture, float2 uv,
												 float weight, int2 x_x1) const {
	constexpr uint32_t IAxis = 1 - Axis;
	float2 st;
	st[IAxis]  = weight;

	int2 y_y1;
	st[Axis] = map<Axis>(texture, uv[Axis], y_y1);

	int4 xy_xy1;
	xy_xy1[IAxis]	  = x_x1[0];
	xy_xy1[Axis]	  = y_y1[0];
	xy_xy1[IAxis + 2] = x_x1[1];
	xy_xy1[Axis  + 2] = y_y1[1];

	float2 c[4];
	texture.gather_2(xy_xy1, c);

	return bilinear(c, st[0], st[1]);
}

template<typename Address_U, typename Address_V>
float3 Linear_2D<Address_U, Address_V>::sample_3(const Texture& texture, float2 uv) const {
	int4 xy_xy1;
	float2 st = map(texture, uv, xy_xy1);

	float3 c[4];
	texture.gather_3(xy_xy1, c);

	return bilinear(c, st[0], st[1]);
}

template<typename Address_U, typename Address_V>
float Linear_2D<Address_U, Address_V>::sample_1(const Texture& texture, float2 uv,
												int32_t element) const {
	int4 xy_xy1;
	float2 st = map(texture, uv, xy_xy1);

	int32_t min_element = std::min(texture.num_elements() - 1, element);

	float c00 = texture.at_element_1(xy_xy1[0], xy_xy1[1], min_element);
	float c01 = texture.at_element_1(xy_xy1[0], xy_xy1[3], min_element);
	float c10 = texture.at_element_1(xy_xy1[2], xy_xy1[1], min_element);
	float c11 = texture.at_element_1(xy_xy1[2], xy_xy1[3], min_element);

	return bilinear(c00, c01, c10, c11, st[0], st[1]);
}

template<typename Address_U, typename Address_V>
float2 Linear_2D<Address_U, Address_V>::sample_2(const Texture& texture, float2 uv,
												 int32_t element) const {
	int4 xy_xy1;
	float2 st = map(texture, uv, xy_xy1);

	int32_t min_element = std::min(texture.num_elements() - 1, element);

	float2 c00 = texture.at_element_2(xy_xy1[0], xy_xy1[1], min_element);
	float2 c01 = texture.at_element_2(xy_xy1[0], xy_xy1[3], min_element);
	float2 c10 = texture.at_element_2(xy_xy1[2], xy_xy1[1], min_element);
	float2 c11 = texture.at_element_2(xy_xy1[2], xy_xy1[3], min_element);

	return bilinear(c00, c01, c10, c11, st[0], st[1]);
}

template<typename Address_U, typename Address_V>
float3 Linear_2D<Address_U, Address_V>::sample_3(const Texture& texture, float2 uv,
												 int32_t element) const {
	int4 xy_xy1;
	float2 st = map(texture, uv, xy_xy1);

	int32_t min_element = std::min(texture.num_elements() - 1, element);

	float3 c00 = texture.at_element_3(xy_xy1[0], xy_xy1[1], min_element);
	float3 c01 = texture.at_element_3(xy_xy1[0], xy_xy1[3], min_element);
	float3 c10 = texture.at_element_3(xy_xy1[2], xy_xy1[1], min_element);
	float3 c11 = texture.at_element_3(xy_xy1[2], xy_xy1[3], min_element);

	return bilinear(c00, c01, c10, c11, st[0], st[1]);
}

template<typename Address_U, typename Address_V>
float2 Linear_2D<Address_U, Address_V>::address(float2 uv) const {
	return float2(Address_U::f(uv[0]), Address_V::f(uv[1]));
}


template<typename Address_U, typename Address_V>
template<uint32_t Axis>
float Linear_2D<Address_U, Address_V>::map(const Texture& texture, float tc, int2& x_x1) {
	const auto b = texture.back_2();
	const auto d = texture.dimensions_float2();

	const float u = Address_U::f(tc) * d[Axis] - 0.5f;

	const float fu = std::floor(u);

	const int32_t x = static_cast<int32_t>(fu);

	x_x1[0] = Address_U::lower_bound(x, b[Axis]);
	x_x1[1] = Address_U::increment(x, b[Axis]);

	return u - fu;
}

template<typename Address_U, typename Address_V>
float2 Linear_2D<Address_U, Address_V>::map(const Texture& texture, float2 uv, int4& xy_xy1) {
	const auto b = texture.back_2();
	const auto d = texture.dimensions_float2();

	const float u = Address_U::f(uv[0]) * d[0] - 0.5f;
	const float v = Address_V::f(uv[1]) * d[1] - 0.5f;

	const float fu = std::floor(u);
	const float fv = std::floor(v);

	const int32_t x = static_cast<int32_t>(fu);
	const int32_t y = static_cast<int32_t>(fv);

	xy_xy1[0] = Address_U::lower_bound(x, b[0]);
	xy_xy1[1] = Address_V::lower_bound(y, b[1]);
	xy_xy1[2] = Address_U::increment(x, b[0]);
	xy_xy1[3] = Address_V::increment(y, b[1]);

	return float2(u - fu, v - fv);
}

}}}
