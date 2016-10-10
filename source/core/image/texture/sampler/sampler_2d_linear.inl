#pragma once

#include "sampler_2d_linear.hpp"
#include "image/texture/texture.hpp"
#include <algorithm>

namespace image { namespace texture { namespace sampler {

inline float bilinear(float c00, float c01, float c10, float c11, float s, float t) {
	float _s = 1.f - s;
	float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

inline float2 bilinear(float2 c00, float2 c01, float2 c10, float2 c11,
					   float s, float t) {
	float _s = 1.f - s;
	float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

inline float3 bilinear(float3_p c00, float3_p c01, float3_p c10, float3_p c11,
					   float s, float t) {
	float _s = 1.f - s;
	float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

inline float4 bilinear(const float4& c00, const float4& c01, const float4& c10, const float4& c11,
					   float s, float t) {
	float _s = 1.f - s;
	float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

template<typename Address_mode>
float Sampler_2D_linear<Address_mode>::sample_1(const Texture& texture, float2 uv) const {
	int4 xy_xy1;
	float2 st = map(texture, uv, xy_xy1);

	float c00 = texture.at_1(xy_xy1.x, xy_xy1.y);
	float c01 = texture.at_1(xy_xy1.x, xy_xy1.w);
	float c10 = texture.at_1(xy_xy1.z, xy_xy1.y);
	float c11 = texture.at_1(xy_xy1.z, xy_xy1.w);

	return bilinear(c00, c01, c10, c11, st.x, st.y);
}

template<typename Address_mode>
float2 Sampler_2D_linear<Address_mode>::sample_2(const Texture& texture, float2 uv) const {
	int4 xy_xy1;
	float2 st = map(texture, uv, xy_xy1);

	float2 c00 = texture.at_2(xy_xy1.x, xy_xy1.y);
	float2 c01 = texture.at_2(xy_xy1.x, xy_xy1.w);
	float2 c10 = texture.at_2(xy_xy1.z, xy_xy1.y);
	float2 c11 = texture.at_2(xy_xy1.z, xy_xy1.w);

	return bilinear(c00, c01, c10, c11, st.x, st.y);
}

template<typename Address_mode>
float3 Sampler_2D_linear<Address_mode>::sample_3(const Texture& texture, float2 uv) const {
	int4 xy_xy1;
	float2 st = map(texture, uv, xy_xy1);

	float3 c00 = texture.at_3(xy_xy1.x, xy_xy1.y);
	float3 c01 = texture.at_3(xy_xy1.x, xy_xy1.w);
	float3 c10 = texture.at_3(xy_xy1.z, xy_xy1.y);
	float3 c11 = texture.at_3(xy_xy1.z, xy_xy1.w);

	return bilinear(c00, c01, c10, c11, st.x, st.y);
}

template<typename Address_mode>
float Sampler_2D_linear<Address_mode>::sample_1(const Texture& texture, float2 uv,
												int32_t element) const {
	int4 xy_xy1;
	float2 st = map(texture, uv, xy_xy1);

	int32_t min_element = std::min(texture.num_elements() - 1, element);

	float c00 = texture.at_element_1(xy_xy1.x, xy_xy1.y, min_element);
	float c01 = texture.at_element_1(xy_xy1.x, xy_xy1.w, min_element);
	float c10 = texture.at_element_1(xy_xy1.z, xy_xy1.y, min_element);
	float c11 = texture.at_element_1(xy_xy1.z, xy_xy1.w, min_element);

	return bilinear(c00, c01, c10, c11, st.x, st.y);
}

template<typename Address_mode>
float2 Sampler_2D_linear<Address_mode>::sample_2(const Texture& texture, float2 uv,
												 int32_t element) const {
	int4 xy_xy1;
	float2 st = map(texture, uv, xy_xy1);

	int32_t min_element = std::min(texture.num_elements() - 1, element);

	float2 c00 = texture.at_element_2(xy_xy1.x, xy_xy1.y, min_element);
	float2 c01 = texture.at_element_2(xy_xy1.x, xy_xy1.w, min_element);
	float2 c10 = texture.at_element_2(xy_xy1.z, xy_xy1.y, min_element);
	float2 c11 = texture.at_element_2(xy_xy1.z, xy_xy1.w, min_element);

	return bilinear(c00, c01, c10, c11, st.x, st.y);
}

template<typename Address_mode>
float3 Sampler_2D_linear<Address_mode>::sample_3(const Texture& texture, float2 uv,
												 int32_t element) const {
	int4 xy_xy1;
	float2 st = map(texture, uv, xy_xy1);

	int32_t min_element = std::min(texture.num_elements() - 1, element);

	float3 c00 = texture.at_element_3(xy_xy1.x, xy_xy1.y, min_element);
	float3 c01 = texture.at_element_3(xy_xy1.x, xy_xy1.w, min_element);
	float3 c10 = texture.at_element_3(xy_xy1.z, xy_xy1.y, min_element);
	float3 c11 = texture.at_element_3(xy_xy1.z, xy_xy1.w, min_element);

	return bilinear(c00, c01, c10, c11, st.x, st.y);
}

template<typename Address_mode>
float2 Sampler_2D_linear<Address_mode>::address(float2 uv) const {
	return Address_mode::f(uv);
}

template<typename Address_mode>
float2 Sampler_2D_linear<Address_mode>::map(const Texture& texture, float2 uv, int4& xy_xy1) {
	auto b = texture.back_2();
	auto d = texture.dimensions_float2();

	uv = Address_mode::f(uv);

	float u = uv.x * d.x - 0.5f;
	float v = uv.y * d.y - 0.5f;

	float fu = std::floor(u);
	float fv = std::floor(v);

	int32_t x = static_cast<int32_t>(fu);
	int32_t y = static_cast<int32_t>(fv);

	xy_xy1.x = Address_mode::lower_bound(x, b.x);
	xy_xy1.y = Address_mode::lower_bound(y, b.y);
	xy_xy1.z = Address_mode::increment(x, b.x);
	xy_xy1.w = Address_mode::increment(y, b.y);

	return float2(u - fu, v - fv);
}

}}}
