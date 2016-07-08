#pragma once

#include "sampler_2d_linear.hpp"
#include "image/texture/texture_2d.hpp"
#include <algorithm>

namespace image { namespace texture { namespace sampler {

inline float bilinear_1(float c00, float c01, float c10, float c11, float s, float t);

inline float2 bilinear_2(float2 c00, float2 c01,
						 float2 c10, float2 c11,
						 float s, float t);

template<typename T>
T bilinear(const T& c00, const T& c01, const T& c10, const T& c11, float s, float t);

template<typename Address_mode>
float Sampler_2D_linear<Address_mode>::sample_1(const Texture_2D& texture, float2 uv) const {
	auto d  = texture.dimensions();
	auto df = texture.dimensions_float();

	uv = Address_mode::f(uv);

	float u = uv.x * df.x - 0.5f;
	float v = uv.y * df.y - 0.5f;

	float fu = std::floor(u);
	float fv = std::floor(v);

	int32_t x = static_cast<int32_t>(fu);
	int32_t y = static_cast<int32_t>(fv);

	int2 m = int2(d.x - 1, d.y - 1);
	int32_t x1 = Address_mode::increment(x, m.x);
	int32_t y1 = Address_mode::increment(y, m.y);
	x = Address_mode::lower_bound(x, m.x);
	y = Address_mode::lower_bound(y, m.y);

	float c00 = texture.at_1(x,  y);
	float c01 = texture.at_1(x,  y1);
	float c10 = texture.at_1(x1, y);
	float c11 = texture.at_1(x1, y1);

	float s = u - fu;
	float t = v - fv;

	return bilinear_1(c00, c01, c10, c11, s, t);
}

template<typename Address_mode>
float2 Sampler_2D_linear<Address_mode>::sample_2(const Texture_2D& texture, float2 uv) const {
	auto d  = texture.dimensions();
	auto df = texture.dimensions_float();

	uv = Address_mode::f(uv);

	float u = uv.x * df.x - 0.5f;
	float v = uv.y * df.y - 0.5f;

	float fu = std::floor(u);
	float fv = std::floor(v);

	int32_t x = static_cast<int32_t>(fu);
	int32_t y = static_cast<int32_t>(fv);

	int2 m = int2(d.x - 1, d.y - 1);
	int32_t x1 = Address_mode::increment(x, m.x);
	int32_t y1 = Address_mode::increment(y, m.y);
	x = Address_mode::lower_bound(x, m.x);
	y = Address_mode::lower_bound(y, m.y);

	float2 c00 = texture.at_2(x,  y);
	float2 c01 = texture.at_2(x,  y1);
	float2 c10 = texture.at_2(x1, y);
	float2 c11 = texture.at_2(x1, y1);

	float s = u - fu;
	float t = v - fv;

	return bilinear_2(c00, c01, c10, c11, s, t);
}

template<typename Address_mode>
float3 Sampler_2D_linear<Address_mode>::sample_3(const Texture_2D& texture, float2 uv) const {
	auto d  = texture.dimensions();
	auto df = texture.dimensions_float();

	uv = Address_mode::f(uv);

	float u = uv.x * df.x - 0.5f;
	float v = uv.y * df.y - 0.5f;

	float fu = std::floor(u);
	float fv = std::floor(v);

	int32_t x = static_cast<int32_t>(fu);
	int32_t y = static_cast<int32_t>(fv);

	int2 m = int2(d.x - 1, d.y - 1);
	int32_t x1 = Address_mode::increment(x, m.x);
	int32_t y1 = Address_mode::increment(y, m.y);
	x = Address_mode::lower_bound(x, m.x);
	y = Address_mode::lower_bound(y, m.y);

	float3 c00 = texture.at_3(x,  y);
	float3 c01 = texture.at_3(x,  y1);
	float3 c10 = texture.at_3(x1, y);
	float3 c11 = texture.at_3(x1, y1);

	float s = u - fu;
	float t = v - fv;

	return bilinear(c00, c01, c10, c11, s, t);
}

template<typename Address_mode>
float Sampler_2D_linear<Address_mode>::sample_1(const Texture_2D& texture, float2 uv,
												int32_t element) const {
	auto d  = texture.dimensions();
	auto df = texture.dimensions_float();

	uv = Address_mode::f(uv);

	float u = uv.x * df.x - 0.5f;
	float v = uv.y * df.y - 0.5f;

	float fu = std::floor(u);
	float fv = std::floor(v);

	int32_t x = static_cast<int32_t>(fu);
	int32_t y = static_cast<int32_t>(fv);

	int2 m = int2(d.x - 1, d.y - 1);
	int32_t x1 = Address_mode::increment(x, m.x);
	int32_t y1 = Address_mode::increment(y, m.y);
	x = Address_mode::lower_bound(x, m.x);
	y = Address_mode::lower_bound(y, m.y);

	int32_t min_element = std::min(texture.num_elements() - 1, element);

	float c00 = texture.at_1(x,  y,  min_element);
	float c01 = texture.at_1(x,  y1, min_element);
	float c10 = texture.at_1(x1, y,  min_element);
	float c11 = texture.at_1(x1, y1, min_element);

	float s = u - fu;
	float t = v - fv;

	return bilinear_1(c00, c01, c10, c11, s, t);
}

template<typename Address_mode>
float2 Sampler_2D_linear<Address_mode>::sample_2(const Texture_2D& texture, float2 uv,
												 int32_t element) const {
	auto d  = texture.dimensions();
	auto df = texture.dimensions_float();

	uv = Address_mode::f(uv);

	float u = uv.x * df.x - 0.5f;
	float v = uv.y * df.y - 0.5f;

	float fu = std::floor(u);
	float fv = std::floor(v);

	int32_t x = static_cast<int32_t>(fu);
	int32_t y = static_cast<int32_t>(fv);

	int2 m = int2(d.x - 1, d.y - 1);
	int32_t x1 = Address_mode::increment(x, m.x);
	int32_t y1 = Address_mode::increment(y, m.y);
	x = Address_mode::lower_bound(x, m.x);
	y = Address_mode::lower_bound(y, m.y);

	int32_t min_element = std::min(texture.num_elements() - 1, element);

	float2 c00 = texture.at_2(x,  y,  min_element);
	float2 c01 = texture.at_2(x,  y1, min_element);
	float2 c10 = texture.at_2(x1, y,  min_element);
	float2 c11 = texture.at_2(x1, y1, min_element);

	float s = u - fu;
	float t = v - fv;

	return bilinear_2(c00, c01, c10, c11, s, t);
}

template<typename Address_mode>
float3 Sampler_2D_linear<Address_mode>::sample_3(const Texture_2D& texture, float2 uv,
												 int32_t element) const {
	auto d  = texture.dimensions();
	auto df = texture.dimensions_float();

	uv = Address_mode::f(uv);

	float u = uv.x * df.x - 0.5f;
	float v = uv.y * df.y - 0.5f;

	float fu = std::floor(u);
	float fv = std::floor(v);

	int32_t x = static_cast<int32_t>(fu);
	int32_t y = static_cast<int32_t>(fv);

	int2 m = int2(d.x - 1, d.y - 1);
	int32_t x1 = Address_mode::increment(x, m.x);
	int32_t y1 = Address_mode::increment(y, m.y);
	x = Address_mode::lower_bound(x, m.x);
	y = Address_mode::lower_bound(y, m.y);

	int32_t min_element = std::min(texture.num_elements() - 1, element);

	float3 c00 = texture.at_3(x,  y,  min_element);
	float3 c01 = texture.at_3(x,  y1, min_element);
	float3 c10 = texture.at_3(x1, y,  min_element);
	float3 c11 = texture.at_3(x1, y1, min_element);

	float s = u - fu;
	float t = v - fv;

	return bilinear(c00, c01, c10, c11, s, t);
}

template<typename Address_mode>
float2 Sampler_2D_linear<Address_mode>::address(float2 uv) const {
	return Address_mode::f(uv);
}

inline float bilinear_1(float c00, float c01, float c10, float c11, float s, float t) {
	float _s = 1.f - s;
	float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

inline float2 bilinear_2(float2 c00, float2 c01,
						 float2 c10, float2 c11,
						 float s, float t) {
	float _s = 1.f - s;
	float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

template<typename T>
T bilinear(const T& c00, const T& c01, const T& c10, const T& c11, float s, float t) {
	float _s = 1.f - s;
	float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

}}}

